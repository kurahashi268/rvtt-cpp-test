# RVTT Sample Application

This is a sample WPF application demonstrating how to integrate the RVTT (Real-Time Voice Transcription Tool) C++ application into a C# desktop application.

## Building the Sample

### Prerequisites

- .NET 6.0 SDK or later
- Windows OS (for WPF)
- Built RVTT executable (`rvtt.exe`)
- Whisper model file (e.g., `ggml-base.bin`)

### Build Instructions

1. Navigate to the SampleApp directory:
```bash
cd csharp/SampleApp
```

2. Build the application:
```bash
dotnet build
```

3. Run the application:
```bash
dotnet run
```

Or open the `.csproj` file in Visual Studio and build/run from there.

## Using the Application

1. **Configure Paths**:
   - Set the **Executable Path** to your compiled `rvtt.exe` (or `rvtt` on Linux)
   - Set the **Model Path** to your Whisper model file
   - Select the **Language** for transcription

2. **Initialize**:
   - Click the "Initialize" button to start the RVTT process
   - Wait for the status to change to "Ready"

3. **Start Listening**:
   - Click "Start Listening" to begin capturing audio
   - Speak into your microphone
   - Transcribed text will appear in the text box

4. **Stop Listening**:
   - Click "Stop Listening" to pause transcription
   - You can resume by clicking "Start Listening" again

5. **Clear**:
   - Click "Clear" to clear the transcription text box

## Features

- Real-time audio transcription
- Start/Stop listening controls
- Visual status indicators
- Clean, modern UI
- Automatic cleanup on exit

## Architecture

The application uses:
- **Shared Memory**: For transferring transcription results from C++ to C#
- **Named Events**: For synchronizing commands between C# and C++ processes
- **Polling**: C# polls the shared memory for updates every 50ms

## Troubleshooting

### "Failed to initialize RVTT"
- Check that the executable path is correct
- Verify that the model file exists
- Ensure you have necessary permissions

### "No transcription appearing"
- Check your microphone is working
- Verify the microphone is set as the default input device
- Try adjusting microphone volume

### "Process terminated unexpectedly"
- Check the C++ application logs
- Verify all dependencies are installed
- Try running in test mode first: `rvtt.exe model.bin ja --test`

