# IPC Integration Guide

This guide explains how to integrate the RVTT (Real-Time Voice Transcription Tool) C++ application with your own applications using the IPC (Inter-Process Communication) interface.

## Overview

RVTT supports two modes of operation:

1. **Test Mode** (`--test` flag): Runs as a standalone CLI application
2. **Main Mode** (default): Runs as a background process communicating via shared memory and events

## Architecture

### Communication Mechanism

RVTT uses two IPC mechanisms:

1. **Shared Memory**: For transferring transcription data from C++ to your application
2. **Named Events/Semaphores**: For synchronizing commands between processes

### IPC Resources

| Resource Name | Type | Purpose |
|---------------|------|---------|
| `RVTTSharedMemory` | Shared Memory | Contains transcription data |
| `RVTTSharedMemory_Ready` | Event | Signaled when RVTT is ready |
| `RVTTSharedMemory_StartListen` | Event | Command to start listening |
| `RVTTSharedMemory_StopListen` | Event | Command to stop listening |
| `RVTTSharedMemory_Quit` | Event | Command to quit RVTT |
| `RVTTSharedMemory_Terminated` | Event | Signaled when RVTT has terminated |

### Shared Memory Structure

```cpp
struct TranscriptionData {
    char text[4096];           // UTF-8 encoded transcription text
    int64_t timestamp;         // Unix timestamp in milliseconds
    bool is_final;             // Whether this is a final result
    int sequence_number;       // Incremental sequence number
};
```

## Communication Flow

### 1. Initialization

1. Your application creates/opens the shared memory and events
2. Your application starts the RVTT process: `rvtt <model_path> <language>`
3. RVTT initializes and signals `Ready` event
4. Your application waits for the `Ready` event (with timeout)

### 2. Start Listening

1. Your application signals the `StartListen` event
2. RVTT starts capturing audio and transcribing
3. RVTT writes transcription results to shared memory
4. Your application polls shared memory for updates (check `sequence_number`)

### 3. Stop Listening

1. Your application signals the `StopListen` event
2. RVTT pauses audio capture (transcription engine remains loaded)
3. Can be resumed by signaling `StartListen` again

### 4. Shutdown

1. Your application signals the `Quit` event
2. RVTT cleans up resources
3. RVTT signals `Terminated` event
4. Your application waits for `Terminated` event
5. Your application cleans up IPC resources

## Platform Specifics

### Windows

- **Shared Memory**: Uses `CreateFileMapping` and `MapViewOfFile`
- **Events**: Uses `CreateEvent`, `SetEvent`, `WaitForSingleObject`
- **Event Type**: Manual reset events

### Linux

- **Shared Memory**: Uses POSIX shared memory (`shm_open`, `mmap`)
- **Events**: Uses POSIX semaphores (`sem_open`, `sem_post`, `sem_wait`)
- **Naming**: Event names are prefixed with `/` on Linux

## C# Integration

### Using the RVTTClient Class

The provided `RVTTClient.cs` class wraps all the complexity:

```csharp
using RVTTClient;

// Create instance
var transcriber = new RVTTTranscriber();

// Subscribe to events
transcriber.TranscriptionReceived += (s, e) => 
{
    Console.WriteLine($"Transcribed: {e.Text}");
};

// Start the process
await transcriber.StartAsync(
    executablePath: "./rvtt.exe",
    modelPath: "./models/ggml-base.bin",
    language: "ja",
    timeout: 30000
);

// Start listening
transcriber.StartListening();

// Stop listening
transcriber.StopListening();

// Cleanup
transcriber.Dispose();
```

### Events

```csharp
// Process is ready to receive commands
transcriber.ProcessReady += (s, e) => { };

// New transcription data available
transcriber.TranscriptionReceived += (s, e) => 
{
    Console.WriteLine($"Text: {e.Text}");
    Console.WriteLine($"Time: {e.Timestamp}");
    Console.WriteLine($"Seq: {e.SequenceNumber}");
};

// Process has terminated
transcriber.ProcessTerminated += (s, e) => { };
```

## C++ Integration

If you want to integrate RVTT into another C++ application, you can use the `IPCManager` class directly:

```cpp
#include "ipc_manager.h"

// Create IPC manager
IPCManager ipc("RVTTSharedMemory");
ipc.initialize();

// Start RVTT process
system("rvtt model.bin ja &");

// Wait for ready
if (ipc.waitForEvent(IPCEvent::READY, 30000)) {
    // Start listening
    ipc.signalEvent(IPCEvent::START_LISTEN);
    
    // Poll for transcriptions
    while (running) {
        TranscriptionData data;
        if (ipc.readTranscription(data)) {
            std::cout << data.text << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Quit
    ipc.signalEvent(IPCEvent::QUIT);
    ipc.waitForEvent(IPCEvent::TERMINATED, 5000);
}
```

## Python Integration

Here's a basic example for Python:

```python
import mmap
import struct
import subprocess
from multiprocessing import Event
import threading

class RVTTClient:
    def __init__(self):
        self.shm = None
        self.process = None
        
    def start(self, executable_path, model_path, language='ja'):
        # Create shared memory (platform-specific)
        # Create events (platform-specific)
        # Start process
        self.process = subprocess.Popen([executable_path, model_path, language])
        # Wait for ready event
        
    def read_transcription(self):
        # Read from shared memory
        # Return text, timestamp, is_final, sequence_number
        pass
        
    def start_listening(self):
        # Signal start event
        pass
        
    def stop_listening(self):
        # Signal stop event
        pass
        
    def close(self):
        # Signal quit event
        # Wait for terminated event
        # Cleanup
        pass
```

## Best Practices

1. **Always wait for the Ready event** before sending commands
2. **Poll shared memory** at 50-100ms intervals (don't poll too frequently)
3. **Check sequence_number** to detect new transcriptions
4. **Handle timeouts** when waiting for events (don't wait forever)
5. **Clean up properly** by signaling Quit and waiting for Terminated
6. **Handle process crashes** by monitoring the process handle
7. **Use UTF-8 encoding** when reading text from shared memory

## Error Handling

### Common Issues

1. **Process doesn't start**: Check executable path and permissions
2. **Ready event timeout**: Check model file exists and is valid
3. **No transcriptions**: Check audio device and permissions
4. **Process crash**: Check logs, verify model compatibility

### Debugging

Run RVTT in test mode first to verify it works:
```bash
rvtt model.bin ja --test
```

This will run RVTT as a CLI application where you can see output directly.

## Performance Considerations

- **Shared memory updates**: RVTT writes every 3 seconds (configurable)
- **Polling frequency**: Recommended 50-100ms
- **Memory usage**: ~4KB for shared memory
- **CPU usage**: Depends on model size and audio activity

## Security Considerations

- Shared memory is readable by any process with appropriate permissions
- Consider using access control lists (ACLs) on Windows
- On Linux, use appropriate permissions on shared memory and semaphores
- Don't store sensitive data in transcriptions if security is a concern

## Examples

See the following examples in the repository:
- `csharp/SampleApp/` - Full WPF application
- `csharp/SimpleConsoleExample.cs` - Simple console example
- Test mode - Run with `--test` flag for standalone operation

