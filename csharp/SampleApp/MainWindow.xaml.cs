using System;
using System.IO;
using System.Windows;
using System.Windows.Media;
using Microsoft.Win32;
using RVTTClient;

namespace RVTTSampleApp
{
    public partial class MainWindow : Window
    {
        private RVTTTranscriber _transcriber;
        private bool _isInitialized = false;

        public MainWindow()
        {
            InitializeComponent();
        }

        private async void Initialize_Click(object sender, RoutedEventArgs e)
        {
            if (_isInitialized)
            {
                MessageBox.Show("Already initialized. Please stop first.", "Information", 
                    MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            string executablePath = txtExecutablePath.Text;
            string modelPath = txtModelPath.Text;
            string language = GetSelectedLanguage();

            // Validate paths
            if (!File.Exists(executablePath))
            {
                MessageBox.Show($"Executable not found: {executablePath}", "Error", 
                    MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            if (!File.Exists(modelPath))
            {
                MessageBox.Show($"Model file not found: {modelPath}", "Error", 
                    MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            // Disable initialization button
            btnInitialize.IsEnabled = false;
            UpdateStatus("Initializing...", Colors.Orange);
            UpdateInfo("Starting RVTT process...");

            try
            {
                // Create and initialize transcriber
                _transcriber = new RVTTTranscriber();
                
                // Subscribe to events
                _transcriber.ProcessReady += Transcriber_ProcessReady;
                _transcriber.ProcessTerminated += Transcriber_ProcessTerminated;
                _transcriber.TranscriptionReceived += Transcriber_TranscriptionReceived;

                // Start the process
                bool success = await _transcriber.StartAsync(executablePath, modelPath, language, 30000);

                if (success)
                {
                    _isInitialized = true;
                    btnStartListening.IsEnabled = true;
                    UpdateStatus("Ready", Colors.Green);
                    UpdateInfo("RVTT engine is ready. Click 'Start Listening' to begin transcription.");
                }
                else
                {
                    MessageBox.Show("Failed to initialize RVTT. Please check the paths and try again.", 
                        "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                    btnInitialize.IsEnabled = true;
                    UpdateStatus("Failed", Colors.Red);
                    UpdateInfo("Failed to start RVTT process. Please check configuration.");
                    _transcriber?.Dispose();
                    _transcriber = null;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error initializing RVTT: {ex.Message}", "Error", 
                    MessageBoxButton.OK, MessageBoxImage.Error);
                btnInitialize.IsEnabled = true;
                UpdateStatus("Error", Colors.Red);
                UpdateInfo($"Error: {ex.Message}");
                _transcriber?.Dispose();
                _transcriber = null;
            }
        }

        private void StartListening_Click(object sender, RoutedEventArgs e)
        {
            if (_transcriber == null || !_isInitialized)
                return;

            try
            {
                _transcriber.StartListening();
                btnStartListening.IsEnabled = false;
                btnStopListening.IsEnabled = true;
                UpdateListeningIndicator(true);
                UpdateStatus("Listening", Colors.Blue);
                UpdateInfo("Listening for audio input. Speak into your microphone...");
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error starting listening: {ex.Message}", "Error", 
                    MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void StopListening_Click(object sender, RoutedEventArgs e)
        {
            if (_transcriber == null || !_isInitialized)
                return;

            try
            {
                _transcriber.StopListening();
                btnStartListening.IsEnabled = true;
                btnStopListening.IsEnabled = false;
                UpdateListeningIndicator(false);
                UpdateStatus("Paused", Colors.Orange);
                UpdateInfo("Listening paused. Click 'Start Listening' to resume.");
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error stopping listening: {ex.Message}", "Error", 
                    MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void Clear_Click(object sender, RoutedEventArgs e)
        {
            txtTranscription.Clear();
        }

        private void BrowseExecutable_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog
            {
                Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*",
                Title = "Select RVTT Executable"
            };

            if (dialog.ShowDialog() == true)
            {
                txtExecutablePath.Text = dialog.FileName;
            }
        }

        private void BrowseModel_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog
            {
                Filter = "Model files (*.bin)|*.bin|All files (*.*)|*.*",
                Title = "Select Whisper Model File"
            };

            if (dialog.ShowDialog() == true)
            {
                txtModelPath.Text = dialog.FileName;
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            // Clean up transcriber
            if (_transcriber != null)
            {
                UpdateStatus("Shutting down...", Colors.Orange);
                _transcriber.Dispose();
                _transcriber = null;
            }
        }

        private void Transcriber_ProcessReady(object sender, EventArgs e)
        {
            Dispatcher.Invoke(() =>
            {
                UpdateInfo("RVTT process is ready and waiting for commands.");
            });
        }

        private void Transcriber_ProcessTerminated(object sender, EventArgs e)
        {
            Dispatcher.Invoke(() =>
            {
                _isInitialized = false;
                btnInitialize.IsEnabled = true;
                btnStartListening.IsEnabled = false;
                btnStopListening.IsEnabled = false;
                UpdateStatus("Terminated", Colors.Gray);
                UpdateListeningIndicator(false);
                UpdateInfo("RVTT process has terminated.");
            });
        }

        private void Transcriber_TranscriptionReceived(object sender, TranscriptionEventArgs e)
        {
            Dispatcher.Invoke(() =>
            {
                // Append the transcribed text
                if (!string.IsNullOrEmpty(e.Text))
                {
                    txtTranscription.AppendText(e.Text);
                    txtTranscription.AppendText(Environment.NewLine);
                    txtTranscription.ScrollToEnd();

                    UpdateInfo($"Last update: {e.Timestamp:HH:mm:ss} | Sequence: {e.SequenceNumber}");
                }
            });
        }

        private string GetSelectedLanguage()
        {
            if (cmbLanguage.SelectedItem is not System.Windows.Controls.ComboBoxItem item)
                return "ja";

            string content = item.Content.ToString();
            
            // Extract language code from content like "Japanese (ja)"
            int startIndex = content.IndexOf('(');
            int endIndex = content.IndexOf(')');
            
            if (startIndex >= 0 && endIndex > startIndex)
            {
                return content.Substring(startIndex + 1, endIndex - startIndex - 1);
            }

            return "ja"; // Default
        }

        private void UpdateStatus(string status, Color color)
        {
            lblStatus.Content = status;
            lblStatus.Foreground = new SolidColorBrush(color);
        }

        private void UpdateListeningIndicator(bool isListening)
        {
            ellipseListening.Fill = new SolidColorBrush(isListening ? Colors.Red : Colors.Gray);
        }

        private void UpdateInfo(string message)
        {
            txtInfo.Text = message;
        }
    }
}

