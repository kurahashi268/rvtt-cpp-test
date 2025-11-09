# C# Integration for RVTT

This directory contains C# integration code for the RVTT (Real-Time Voice Transcription Tool).

## Contents

- **`RVTTClient.cs`** - Complete C# wrapper class for easy integration
- **`SimpleConsoleExample.cs`** - Minimal console application example
- **`SampleApp/`** - Full-featured WPF desktop application

## Quick Integration

### 1. Copy the Client Class

Copy `RVTTClient.cs` to your C# project.

### 2. Basic Usage

```csharp
using RVTTClient;

// Create the transcriber
var transcriber = new RVTTTranscriber();

// Subscribe to transcription events
transcriber.TranscriptionReceived += (s, e) => 
{
    Console.WriteLine($"Transcribed: {e.Text}");
    Console.WriteLine($"Timestamp: {e.Timestamp}");
    Console.WriteLine($"Sequence: {e.SequenceNumber}");
};

// Start the RVTT process
bool success = await transcriber.StartAsync(
    executablePath: "./rvtt.exe",      // Path to rvtt executable
    modelPath: "./models/ggml-base.bin", // Path to Whisper model
    language: "ja",                     // Language code
    timeout: 30000                      // Timeout in milliseconds
);

if (success)
{
    // Start listening
    transcriber.StartListening();
    
    // Do your work here...
    await Task.Delay(60000); // Listen for 60 seconds
    
    // Stop listening (pause)
    transcriber.StopListening();
    
    // Can resume later
    transcriber.StartListening();
    
    // Cleanup
    transcriber.Dispose();
}
```

## API Reference

### RVTTTranscriber Class

#### Events

- **`ProcessReady`** - Raised when RVTT process is ready to accept commands
- **`ProcessTerminated`** - Raised when RVTT process has terminated
- **`TranscriptionReceived`** - Raised when new transcription text is available

#### Properties

- **`IsListening`** - Gets whether the transcriber is currently listening
- **`IsRunning`** - Gets whether the RVTT process is running

#### Methods

##### StartAsync
```csharp
Task<bool> StartAsync(
    string executablePath,
    string modelPath,
    string language = "ja",
    int timeout = 30000
)
```
Starts the RVTT process and initializes IPC.

**Parameters:**
- `executablePath` - Path to the rvtt executable
- `modelPath` - Path to the Whisper model file
- `language` - Language code (default: "ja")
- `timeout` - Timeout in milliseconds (default: 30000)

**Returns:** `true` if successfully started, `false` otherwise

##### StartListening
```csharp
bool StartListening()
```
Starts capturing audio and transcribing.

**Returns:** `true` if successful

##### StopListening
```csharp
bool StopListening()
```
Stops capturing audio (pauses transcription, engine remains loaded).

**Returns:** `true` if successful

##### Stop
```csharp
void Stop()
```
Stops the RVTT process gracefully.

##### Dispose
```csharp
void Dispose()
```
Releases all resources and stops the process.

### TranscriptionEventArgs

Properties:
- **`Text`** (string) - The transcribed text
- **`Timestamp`** (DateTime) - When the transcription occurred
- **`IsFinal`** (bool) - Whether this is a final result
- **`SequenceNumber`** (int) - Sequence number for tracking updates

## Examples

### Console Application

See `SimpleConsoleExample.cs` for a complete console application example.

```bash
dotnet new console -n MyTranscriber
cd MyTranscriber
# Copy RVTTClient.cs and SimpleConsoleExample.cs
dotnet run ./rvtt.exe ./models/ggml-base.bin ja
```

### WPF Application

See `SampleApp/` for a complete WPF application with:
- Configuration UI
- Start/Stop controls
- Real-time transcription display
- Status indicators
- Error handling

```bash
cd SampleApp
dotnet run
```

## Supported Platforms

- **Windows**: Full support (uses Named Events and File Mapping)
- **Linux**: Limited support (requires Mono or .NET Core with P/Invoke adjustments)
- **.NET Version**: .NET 6.0 or later

## Requirements

### Runtime Requirements

- .NET 6.0 Runtime or later
- Built RVTT executable
- Whisper model file

### Development Requirements

- .NET 6.0 SDK or later
- Visual Studio 2022 or VS Code (optional)

## Architecture

The C# client communicates with the C++ RVTT process via:

1. **Shared Memory** (`MemoryMappedFile`) - For transcription data transfer
2. **Named Events** (`EventWaitHandle`) - For command synchronization

### IPC Resources

| Resource | Type | Purpose |
|----------|------|---------|
| RVTTSharedMemory | Shared Memory | Transcription data |
| RVTTSharedMemory_Ready | Event | Process ready signal |
| RVTTSharedMemory_StartListen | Event | Start listening command |
| RVTTSharedMemory_StopListen | Event | Stop listening command |
| RVTTSharedMemory_Quit | Event | Quit command |
| RVTTSharedMemory_Terminated | Event | Process terminated signal |

## Troubleshooting

### "Failed to start RVTT"

- Verify executable path is correct
- Check that model file exists
- Ensure you have execute permissions

### No transcription events

- Check that microphone is working
- Verify language code is correct
- Ensure `StartListening()` was called

### Process crashes immediately

- Run RVTT in test mode first: `rvtt.exe model.bin ja --test`
- Check console output for errors
- Verify model file is not corrupted

### Access denied errors

- Run with administrator privileges (if needed)
- Check file permissions

## Performance Tips

1. **Polling Rate**: The client polls shared memory every 50ms. Adjust in `PollTranscriptionData()` if needed.
2. **Model Size**: Use smaller models for faster response times
3. **Cleanup**: Always call `Dispose()` to ensure proper cleanup

## Thread Safety

- All public methods are thread-safe
- Events are raised on a background thread, use `Dispatcher.Invoke()` for UI updates in WPF

## License

Same as RVTT main project.

## Support

For detailed integration guidance, see:
- [IPC Integration Guide](../docs/IPC_INTEGRATION_GUIDE.md)
- [Usage Guide](../docs/USAGE_GUIDE.md)

