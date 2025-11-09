# Quick Start Guide

Get started with RVTT in 5 minutes!

## For Testing (CLI Mode)

### 1. Build

```bash
./build.sh
```

### 2. Download Model

```bash
cd models
./download-model.sh base
cd ..
```

### 3. Run

```bash
./build/rvtt models/ggml-base.bin ja --test
```

That's it! Speak into your microphone and see the transcription in real-time.

## For C# Integration

### 1. Build RVTT

```bash
./build.sh
```

### 2. Download Model

```bash
cd models
./download-model.sh base
cd ..
```

### 3. Run the Sample App

```bash
cd csharp/SampleApp
dotnet run
```

Or open `RVTTSampleApp.csproj` in Visual Studio and run.

### 4. In Your Own C# App

1. Copy `csharp/RVTTClient.cs` to your project

2. Add this code:

```csharp
using RVTTClient;

var transcriber = new RVTTTranscriber();
transcriber.TranscriptionReceived += (s, e) => 
{
    Console.WriteLine($"[{e.Timestamp:HH:mm:ss}] {e.Text}");
};

await transcriber.StartAsync("./rvtt", "./models/ggml-base.bin", "en");
transcriber.StartListening();

// Your app runs here...

transcriber.Dispose();
```

## Supported Languages

| Code | Language |
|------|----------|
| `ja` | Japanese |
| `en` | English |
| `zh` | Chinese |
| `ko` | Korean |
| `es` | Spanish |
| `fr` | French |
| `de` | German |

## Model Sizes

| Model | Size | Speed | Quality |
|-------|------|-------|---------|
| tiny | 75 MB | Fastest | Basic |
| base | 142 MB | Fast | Good |
| small | 466 MB | Medium | Better |
| medium | 1.5 GB | Slow | High |

**Recommendation**: Start with `base` for good balance.

## Common Commands

```bash
# Test mode (CLI)
./build/rvtt models/ggml-base.bin en --test

# IPC mode (for C# integration)
./build/rvtt models/ggml-base.bin en

# Build from scratch
./build.sh

# Clean build
rm -rf build && ./build.sh
```

## Troubleshooting

### "Failed to initialize audio capture"

Check your microphone:
```bash
# Linux
arecord -l

# Test recording
arecord -d 3 test.wav && aplay test.wav
```

### "Failed to initialize transcriber"

Make sure model file exists:
```bash
ls -lh models/ggml-base.bin
```

### No output

- Check microphone volume
- Speak clearly and loudly
- Verify correct language code

## Next Steps

- **For detailed usage**: See [docs/USAGE_GUIDE.md](docs/USAGE_GUIDE.md)
- **For integration**: See [docs/IPC_INTEGRATION_GUIDE.md](docs/IPC_INTEGRATION_GUIDE.md)
- **For C# sample**: See [csharp/SampleApp/README.md](csharp/SampleApp/README.md)

## Help

If you encounter issues:

1. Try test mode first: `./build/rvtt models/ggml-base.bin en --test`
2. Check the logs for error messages
3. Verify all prerequisites are installed
4. See the troubleshooting section in the main README

Happy transcribing! 🎤

