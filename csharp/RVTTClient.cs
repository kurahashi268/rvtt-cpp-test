using System;
using System.Diagnostics;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace RVTTClient
{
    /// <summary>
    /// Transcription data structure matching the C++ TranscriptionData
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public unsafe struct TranscriptionData
    {
        public fixed byte Text[4096];

        public long Timestamp;

        [MarshalAs(UnmanagedType.I1)]
        public bool IsFinal;

        public int SequenceNumber;

        /// <summary>
        /// Gets the transcription text as a string
        /// </summary>
        public string GetText()
        {
            fixed (byte* ptr = Text)
            {
                int length = 0;
                while (length < 4096 && ptr[length] != 0)
                {
                    length++;
                }
                return Encoding.UTF8.GetString(ptr, length);
            }
        }
    }

    /// <summary>
    /// Event arguments for transcription updates
    /// </summary>
    public class TranscriptionEventArgs : EventArgs
    {
        public string Text { get; set; }
        public DateTime Timestamp { get; set; }
        public bool IsFinal { get; set; }
        public int SequenceNumber { get; set; }
    }

    /// <summary>
    /// Client for communicating with the RVTT C++ application
    /// </summary>
    public class RVTTTranscriber : IDisposable
    {
        private const string SharedMemoryName = "RVTTSharedMemory";
        private const string EventReadyName = "RVTTSharedMemory_Ready";
        private const string EventStartListenName = "RVTTSharedMemory_StartListen";
        private const string EventStopListenName = "RVTTSharedMemory_StopListen";
        private const string EventTranscriptionCompleteName = "RVTTSharedMemory_TranscriptionComplete";
        private const string EventQuitName = "RVTTSharedMemory_Quit";
        private const string EventTerminatedName = "RVTTSharedMemory_Terminated";

        private Process _process;
        private MemoryMappedFile _sharedMemory;
        private EventWaitHandle _eventReady;
        private EventWaitHandle _eventStartListen;
        private EventWaitHandle _eventStopListen;
        private EventWaitHandle _eventTranscriptionComplete;
        private EventWaitHandle _eventQuit;
        private EventWaitHandle _eventTerminated;

        private CancellationTokenSource _pollingCancellation;
        private Task _pollingTask;
        private int _lastSequenceNumber = -1;

        private bool _isDisposed = false;
        private bool _isListening = false;

        /// <summary>
        /// Event raised when new transcription text is available
        /// </summary>
        public event EventHandler<TranscriptionEventArgs> TranscriptionReceived;

        /// <summary>
        /// Event raised when the RVTT process is ready
        /// </summary>
        public event EventHandler ProcessReady;

        /// <summary>
        /// Event raised when the RVTT process has terminated
        /// </summary>
        public event EventHandler ProcessTerminated;

        /// <summary>
        /// Event raised when transcription is complete (all buffered audio has been processed)
        /// </summary>
        public event EventHandler TranscriptionComplete;

        /// <summary>
        /// Gets whether the transcriber is currently listening
        /// </summary>
        public bool IsListening => _isListening;

        /// <summary>
        /// Gets whether the transcriber is running
        /// </summary>
        public bool IsRunning => _process != null && !_process.HasExited;

        /// <summary>
        /// Starts the RVTT process with the specified model and language
        /// </summary>
        /// <param name="executablePath">Path to the rvtt executable</param>
        /// <param name="modelPath">Path to the Whisper model file</param>
        /// <param name="language">Language code (e.g., "ja", "en")</param>
        /// <param name="timeout">Timeout in milliseconds to wait for the process to be ready</param>
        /// <returns>True if successfully started and ready</returns>
        public async Task<bool> StartAsync(string executablePath, string modelPath, string language = "ja", int timeout = 30000)
        {
            if (_isDisposed)
                throw new ObjectDisposedException(nameof(RVTTTranscriber));

            if (_process != null)
            {
                throw new InvalidOperationException("Process is already running");
            }

            try
            {
                // Initialize shared memory and events
                if (!InitializeIPC())
                {
                    return false;
                }

                // Start the process
                var startInfo = new ProcessStartInfo
                {
                    FileName = executablePath,
                    Arguments = $"\"{modelPath}\" {language}",
                    UseShellExecute = false,
                    CreateNoWindow = false,
                    RedirectStandardOutput = false,
                    RedirectStandardError = false
                };

                _process = Process.Start(startInfo);

                if (_process == null)
                {
                    CleanupIPC();
                    return false;
                }

                // Wait for the ready event
                bool ready = await Task.Run(() => _eventReady.WaitOne(timeout));

                if (!ready)
                {
                    Stop();
                    return false;
                }

                // Start polling for transcription updates
                _pollingCancellation = new CancellationTokenSource();
                _pollingTask = Task.Run(() => PollTranscriptionData(_pollingCancellation.Token));

                // Start monitoring for transcription complete events
                _ = Task.Run(() => MonitorTranscriptionComplete(_pollingCancellation.Token));

                ProcessReady?.Invoke(this, EventArgs.Empty);

                return true;
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Failed to start RVTT: {ex.Message}");
                Stop();
                return false;
            }
        }

        /// <summary>
        /// Starts listening for audio and transcription
        /// </summary>
        /// <returns>True if successfully started listening</returns>
        public bool StartListening()
        {
            if (_isDisposed)
                throw new ObjectDisposedException(nameof(RVTTTranscriber));

            if (!IsRunning)
                throw new InvalidOperationException("Process is not running");

            if (_isListening)
                return true;

            _eventStartListen.Set();
            _isListening = true;
            return true;
        }

        /// <summary>
        /// Stops listening for audio (pauses transcription)
        /// </summary>
        /// <returns>True if successfully stopped listening</returns>
        public bool StopListening()
        {
            if (_isDisposed)
                throw new ObjectDisposedException(nameof(RVTTTranscriber));

            if (!IsRunning)
                throw new InvalidOperationException("Process is not running");

            if (!_isListening)
                return true;

            _eventStopListen.Set();
            _isListening = false;
            return true;
        }

        /// <summary>
        /// Waits for transcription to complete after stopping listening
        /// </summary>
        /// <param name="timeout">Timeout in milliseconds (-1 for infinite wait)</param>
        /// <returns>True if transcription completed, false if timeout</returns>
        public bool WaitForTranscriptionComplete(int timeout = -1)
        {
            if (_isDisposed)
                throw new ObjectDisposedException(nameof(RVTTTranscriber));

            if (!IsRunning)
                throw new InvalidOperationException("Process is not running");

            if (_eventTranscriptionComplete == null)
                return false;

            int waitTimeout = timeout < 0 ? Timeout.Infinite : timeout;
            bool completed = _eventTranscriptionComplete.WaitOne(waitTimeout);
            
            if (completed)
            {
                // Reset the event after receiving it
                _eventTranscriptionComplete.Reset();
                TranscriptionComplete?.Invoke(this, EventArgs.Empty);
            }
            
            return completed;
        }

        /// <summary>
        /// Stops the RVTT process gracefully
        /// </summary>
        public void Stop()
        {
            if (_isDisposed)
                return;

            try
            {
                // Stop polling
                if (_pollingCancellation != null)
                {
                    _pollingCancellation.Cancel();
                    _pollingTask?.Wait(1000);
                }

                // Signal quit to the process
                if (_eventQuit != null && IsRunning)
                {
                    _eventQuit.Set();

                    // Wait for terminated event
                    bool terminated = _eventTerminated.WaitOne(5000);

                    if (!terminated)
                    {
                        // Force kill if not responding
                        _process?.Kill();
                    }
                }

                _process?.Dispose();
                _process = null;

                ProcessTerminated?.Invoke(this, EventArgs.Empty);
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Error stopping RVTT: {ex.Message}");
            }
            finally
            {
                CleanupIPC();
            }
        }

        /// <summary>
        /// Initializes IPC resources (shared memory and events)
        /// </summary>
        private bool InitializeIPC()
        {
            try
            {
                // Create or open shared memory
                _sharedMemory = MemoryMappedFile.CreateOrOpen(
                    SharedMemoryName,
                    Marshal.SizeOf<TranscriptionData>(),
                    MemoryMappedFileAccess.ReadWrite
                );

                // Create events
                _eventReady = new EventWaitHandle(false, EventResetMode.ManualReset, EventReadyName);
                _eventStartListen = new EventWaitHandle(false, EventResetMode.ManualReset, EventStartListenName);
                _eventStopListen = new EventWaitHandle(false, EventResetMode.ManualReset, EventStopListenName);
                _eventTranscriptionComplete = new EventWaitHandle(false, EventResetMode.ManualReset, EventTranscriptionCompleteName);
                _eventQuit = new EventWaitHandle(false, EventResetMode.ManualReset, EventQuitName);
                _eventTerminated = new EventWaitHandle(false, EventResetMode.ManualReset, EventTerminatedName);

                return true;
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Failed to initialize IPC: {ex.Message}");
                CleanupIPC();
                return false;
            }
        }

        /// <summary>
        /// Cleans up IPC resources
        /// </summary>
        private void CleanupIPC()
        {
            _sharedMemory?.Dispose();
            _sharedMemory = null;

            _eventReady?.Dispose();
            _eventReady = null;

            _eventStartListen?.Dispose();
            _eventStartListen = null;

            _eventStopListen?.Dispose();
            _eventStopListen = null;

            _eventTranscriptionComplete?.Dispose();
            _eventTranscriptionComplete = null;

            _eventQuit?.Dispose();
            _eventQuit = null;

            _eventTerminated?.Dispose();
            _eventTerminated = null;
        }

        /// <summary>
        /// Monitors for transcription complete events
        /// </summary>
        private void MonitorTranscriptionComplete(CancellationToken cancellationToken)
        {
            while (!cancellationToken.IsCancellationRequested)
            {
                try
                {
                    if (_eventTranscriptionComplete != null && _eventTranscriptionComplete.WaitOne(100))
                    {
                        // Reset the event after receiving it
                        _eventTranscriptionComplete.Reset();
                        TranscriptionComplete?.Invoke(this, EventArgs.Empty);
                    }
                }
                catch (Exception ex)
                {
                    Debug.WriteLine($"Error monitoring transcription complete: {ex.Message}");
                }
            }
        }

        /// <summary>
        /// Polls the shared memory for transcription updates
        /// </summary>
        private void PollTranscriptionData(CancellationToken cancellationToken)
        {
            while (!cancellationToken.IsCancellationRequested)
            {
                try
                {
                    var data = ReadTranscriptionData();

                    // Only notify if sequence number changed
                    if (data.SequenceNumber > _lastSequenceNumber)
                    {
                        _lastSequenceNumber = data.SequenceNumber;

                        string text = data.GetText();
                        if (!string.IsNullOrEmpty(text))
                        {
                            var args = new TranscriptionEventArgs
                            {
                                Text = text,
                                Timestamp = DateTimeOffset.FromUnixTimeMilliseconds(data.Timestamp).DateTime,
                                IsFinal = data.IsFinal,
                                SequenceNumber = data.SequenceNumber
                            };

                            TranscriptionReceived?.Invoke(this, args);
                        }
                    }

                    Thread.Sleep(50); // Poll every 50ms
                }
                catch (Exception ex)
                {
                    Debug.WriteLine($"Error polling transcription data: {ex.Message}");
                }
            }
        }

        /// <summary>
        /// Reads transcription data from shared memory
        /// </summary>
        public TranscriptionData ReadTranscriptionData()
        {
            if (_sharedMemory == null)
                return default;

            using (var accessor = _sharedMemory.CreateViewAccessor(0, Marshal.SizeOf<TranscriptionData>()))
            {
                accessor.Read(0, out TranscriptionData data);
                return data;
            }
        }

        /// <summary>
        /// Disposes resources
        /// </summary>
        public void Dispose()
        {
            if (_isDisposed)
                return;

            Stop();
            _isDisposed = true;
        }

        ~RVTTTranscriber() 
        {
            Dispose();
        }
    }
}

