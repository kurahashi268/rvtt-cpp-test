# RVTT Usage Guide

## Overview

RVTT (Real-Time Voice Transcription Tool) is a high-performance audio transcription application built with C++ and Whisper.cpp. It supports two modes of operation:

1. **Test Mode**: Standalone CLI application for testing
2. **Main Mode**: Background service for integration with other applications

## Command Line Usage

### Basic Syntax

```bash
rvtt <model_path> [language] [--test]
```

### Parameters

- `model_path` (required): Path to the Whisper model file (e.g., `models/ggml-base.bin`)
- `language` (optional): Language code (default: `ja` for Japanese)
  - Supported: `ja`, `en`, `zh`, `ko`, `es`, `fr`, `de`, etc.
- `--test` (optional): Run in test mode (CLI mode)

### Examples

#### Test Mode (CLI)

Run as a standalone transcriber with console output:

```bash
# Japanese (default)
./rvtt models/ggml-base.bin ja --test

# English
./rvtt models/ggml-base.bin en --test

# Chinese
./rvtt models/ggml-base.bin zh --test
```

#### Main Mode (IPC Service)

Run as a background service for integration:

```bash
# Japanese
./rvtt models/ggml-base.bin ja

# English
./rvtt models/ggml-base.bin en
```

In main mode, the application:
- Creates shared memory and event synchronization
- Waits for commands from the parent application
- Writes transcription results to shared memory
- Responds to start/stop/quit commands

## Downloading Models

Use the provided scripts to download Whisper models:

### Linux/macOS

```bash
cd models
./download-model.sh base
```

### Windows

```cmd
cd models
download-model.bat base
```

Available models:
- `tiny` - Fastest, lowest quality (~75 MB)
- `base` - Good balance (~142 MB)
- `small` - Better quality (~466 MB)
- `medium` - High quality (~1.5 GB)
- `large` - Best quality (~2.9 GB)

## Integration Examples

### C# Desktop Application

```csharp
using RVTTClient;

var transcriber = new RVTTTranscriber();
transcriber.TranscriptionReceived += (s, e) => 
{
    Console.WriteLine(e.Text);
};

await transcriber.StartAsync("./rvtt.exe", "./models/ggml-base.bin", "ja");
transcriber.StartListening();

// ... do work ...

transcriber.Dispose();
```

See `csharp/SampleApp/` for a complete WPF application example.

### Simple Console Integration

See `csharp/SimpleConsoleExample.cs` for a minimal console example.

## Performance Tuning

### Buffer Size

The default audio buffer processes 3 seconds of audio. To change this, modify in `transcriber.cpp`:

```cpp
buffer_size_threshold_(16000 * 3) // 3 seconds at 16kHz
```

### Thread Count

Adjust the number of threads used by Whisper in `transcriber.cpp`:

```cpp
wparams.n_threads = 4; // Increase for more cores
```

### Model Selection

Choose the appropriate model for your use case:

| Model | Size | Speed | Quality | Use Case |
|-------|------|-------|---------|----------|
| tiny | 75 MB | Fastest | Lowest | Quick testing |
| base | 142 MB | Fast | Good | Real-time apps |
| small | 466 MB | Medium | Better | Most applications |
| medium | 1.5 GB | Slow | High | High-quality needs |
| large | 2.9 GB | Slowest | Best | Offline processing |

## Audio Configuration

### Microphone Selection

RVTT uses the system's default input device. To change:

**Windows**: Control Panel → Sound → Recording → Set Default

**Linux (PulseAudio)**:
```bash
pactl list sources short
pactl set-default-source <source_name>
```

**Linux (ALSA)**:
Edit `~/.asoundrc` to set default input device.

### Sample Rate

RVTT uses 16kHz sample rate (optimal for Whisper). This is hardcoded but can be changed in `audio_capture.cpp`:

```cpp
AudioCapture audio_capture(16000, 512); // 16kHz, 512 samples per buffer
```

## Troubleshooting

### "Failed to initialize transcriber"

**Possible causes:**
- Model file not found or corrupted
- Insufficient memory
- Incompatible model format

**Solutions:**
- Verify model file exists and is valid
- Try a smaller model (e.g., `tiny` or `base`)
- Re-download the model

### "Failed to initialize audio capture"

**Possible causes:**
- No microphone connected
- Microphone in use by another application
- PortAudio not installed

**Solutions:**
- Check microphone connection
- Close other applications using the microphone
- Verify PortAudio installation

### No transcription output

**Possible causes:**
- Microphone volume too low
- Wrong input device selected
- Language mismatch

**Solutions:**
- Increase microphone volume
- Check default input device
- Verify language code matches spoken language

### Poor transcription quality

**Possible causes:**
- Background noise
- Model too small
- Wrong language selected

**Solutions:**
- Use in quiet environment
- Try a larger model (`small` or `medium`)
- Verify correct language code

### High CPU usage

**Possible causes:**
- Large model
- Too many threads
- Continuous processing

**Solutions:**
- Use smaller model
- Reduce thread count
- Increase buffer threshold (process less frequently)

## Advanced Configuration

### Custom Whisper Parameters

Edit `transcriber.cpp` to customize Whisper behavior:

```cpp
wparams.print_realtime = false;     // Print realtime transcription
wparams.print_progress = false;      // Print progress
wparams.print_timestamps = false;    // Print timestamps
wparams.translate = false;           // Translate to English
wparams.no_context = true;          // Disable context
wparams.single_segment = false;     // Force single segment
wparams.suppress_blank = true;      // Suppress blank output
wparams.suppress_nst = true;        // Suppress non-speech tokens
```

### Audio Buffer Configuration

Edit `audio_capture.cpp`:

```cpp
// Frames per buffer (smaller = lower latency, higher CPU)
AudioCapture audio_capture(16000, 512);

// Suggested latency
input_parameters.suggestedLatency = 
    Pa_GetDeviceInfo(input_parameters.device)->defaultLowInputLatency;
```

## System Requirements

### Minimum

- CPU: Intel Core i3 or equivalent
- RAM: 2 GB
- Disk: 200 MB (tiny model)
- OS: Windows 10, Linux (Ubuntu 18.04+), macOS 10.14+

### Recommended

- CPU: Intel Core i5 or equivalent (4+ cores)
- RAM: 4 GB
- Disk: 500 MB (base model)
- OS: Windows 10/11, Linux (Ubuntu 20.04+), macOS 11+

### For Best Performance

- CPU: Intel Core i7 or AMD Ryzen 7 (8+ cores)
- RAM: 8 GB
- Disk: 2 GB (small model)
- GPU: Not required (CPU-only)

## Building from Source

See the main README for build instructions.

## License

See LICENSE file in the repository.

## Support

For issues and questions:
- Check this guide first
- See `docs/IPC_INTEGRATION_GUIDE.md` for integration details
- Run in test mode (`--test`) to verify basic functionality
- Check system logs for error messages

