# Real-time Voice Transcription Tool (C++)

A real-time voice dictation program built with C++, using Whisper.cpp for speech recognition and PortAudio for audio capture. Designed for easy integration with desktop applications via shared memory IPC.

## Features

- 🎤 Real-time audio capture from microphone
- 🗣️ High-quality speech recognition using OpenAI's Whisper model
- 🇯🇵 Excellent Japanese language support (also supports other languages)
- ⚡ Low-latency processing
- 🔄 **Dual mode operation**: CLI test mode and IPC service mode
- 🔗 **C# integration**: Ready-to-use C# wrapper class for desktop apps
- 💾 **Shared memory IPC**: Fast communication with parent applications
- 🎛️ **Event-based control**: Start/stop/quit commands via named events
- 🔧 Built with modern C++ (C++17)
- 🛠️ CMake build system with cross-platform support

## Prerequisites

Before building, make sure you have the following installed:

### System Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    clang \
    git \
    portaudio19-dev \
    libasound2-dev
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake clang git portaudio
```

**macOS:**
```bash
brew install cmake portaudio
```

## Setup

### 1. Clone the Repository

```bash
git clone <your-repo-url>
cd rvtt-cpp
```

### 2. Verify Whisper.cpp Core

The whisper.cpp library is integrated in the `core/` directory.

### 3. Download Whisper Model

Download a Whisper model file. For Japanese transcription, the `base` or `small` model works well:

```bash
mkdir -p models
cd models

# Download base model (recommended for Japanese)
bash ../core/models/download-ggml-model.sh base

# Or download small model (better accuracy, slower)
# bash ../core/models/download-ggml-model.sh small

cd ..
```

Available models (ordered by size/quality):
- `tiny` - Fastest, lower quality
- `base` - Good balance (recommended)
- `small` - Better quality
- `medium` - High quality
- `large` - Best quality (very slow)

## Building

### Using the Build Script

```bash
chmod +x build.sh
./build.sh
```

### Manual Build

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..
```

## Usage

RVTT supports two modes of operation:

### 1. Test Mode (CLI)

Run as a standalone CLI application for testing:

```bash
# Japanese
./build/rvtt models/ggml-base.bin ja --test

# English
./build/rvtt models/ggml-base.bin en --test

# Chinese
./build/rvtt models/ggml-base.bin zh --test

# Korean
./build/rvtt models/ggml-base.bin ko --test
```

**Controls:**
- **Speak into your microphone** - The program will transcribe in real-time
- **Ctrl+C** - Stop and exit

### 2. Main Mode (IPC Service)

Run as a background service for integration with other applications:

```bash
# Japanese
./build/rvtt models/ggml-base.bin ja

# English
./build/rvtt models/ggml-base.bin en
```

In this mode, RVTT:
- Creates shared memory for transcription data
- Waits for commands from the parent application
- Responds to start/stop/quit events
- Writes transcription results to shared memory

### 3. C# Integration

Use the provided C# wrapper class for easy integration:

```csharp
using RVTTClient;

var transcriber = new RVTTTranscriber();
transcriber.TranscriptionReceived += (s, e) => 
{
    Console.WriteLine($"Transcribed: {e.Text}");
};

await transcriber.StartAsync(
    "./build/rvtt", 
    "./models/ggml-base.bin", 
    "ja"
);

transcriber.StartListening();
// ... transcription happens ...
transcriber.StopListening();
transcriber.Dispose();
```

**Example Applications:**
- **WPF Sample App**: See `csharp/SampleApp/` for a complete desktop application
- **Console Example**: See `csharp/SimpleConsoleExample.cs` for minimal usage

**Documentation:**
- `docs/IPC_INTEGRATION_GUIDE.md` - Detailed IPC integration guide
- `docs/USAGE_GUIDE.md` - Complete usage documentation
- `csharp/SampleApp/README.md` - Sample application guide

## How It Works

1. **Audio Capture**: PortAudio captures audio from your default microphone at 16kHz sample rate
2. **Buffering**: Audio is buffered in 3-second chunks for optimal transcription
3. **Processing**: Whisper.cpp processes the audio and generates transcriptions
4. **Output**: Transcriptions are displayed in real-time in the terminal

## Configuration

You can modify the following parameters in the source code:

### Audio Settings (`src/main.cpp`)
- Sample rate: `16000` Hz (required for Whisper)
- Frame buffer size: `512` samples

### Transcription Settings (`src/transcriber.cpp`)
- Buffer threshold: `16000 * 3` (3 seconds)
- Number of threads: `4`

## Troubleshooting

### No Audio Input Device

If you get "No default input device" error:
```bash
# List audio devices
arecord -l

# Test your microphone
arecord -d 5 test.wav
aplay test.wav
```

### Model Not Found

Make sure you've downloaded the model:
```bash
ls -lh models/
# Should show ggml-base.bin or similar
```

### Build Errors

If you encounter build errors:
```bash
# Clean build
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Whisper.cpp Not Found

```bash
# Verify core directory exists
ls -la core/
```

## Performance Tips

1. **Use smaller models** for faster processing:
   - `tiny` or `base` for real-time on older hardware
   - `small` or `medium` for better accuracy on modern hardware

2. **Adjust buffer size** in `transcriber.cpp`:
   - Smaller buffer (1-2s) = faster feedback, less context
   - Larger buffer (3-5s) = better accuracy, more delay

3. **Thread count**: Adjust `wparams.n_threads` in `transcriber.cpp` based on your CPU

## Project Structure

```
rvtt-cpp/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
├── build.sh                # Build script
├── include/                # Header files
│   ├── audio_capture.h     # Audio capture interface
│   ├── transcriber.h       # Transcription interface
│   └── ipc_manager.h       # IPC communication interface
├── src/                    # Source files
│   ├── main.cpp            # Main program (test & IPC modes)
│   ├── audio_capture.cpp   # Audio capture implementation
│   ├── transcriber.cpp     # Transcription implementation
│   └── ipc_manager.cpp     # IPC implementation
├── csharp/                 # C# integration
│   ├── RVTTClient.cs       # C# wrapper class
│   ├── SimpleConsoleExample.cs  # Console example
│   └── SampleApp/          # Full WPF sample application
│       ├── MainWindow.xaml
│       ├── MainWindow.xaml.cs
│       ├── App.xaml
│       └── RVTTSampleApp.csproj
├── docs/                   # Documentation
│   ├── IPC_INTEGRATION_GUIDE.md  # IPC integration guide
│   └── USAGE_GUIDE.md      # Complete usage guide
├── core/                   # Whisper.cpp core library
│   ├── include/            # Whisper headers
│   ├── src/                # Whisper implementation
│   └── ggml/               # GGML backend
└── models/                 # Whisper model files
    ├── download-model.sh   # Model download script
    └── ggml-*.bin          # Downloaded models
```

## License

This project uses:
- [Whisper.cpp](https://github.com/ggerganov/whisper.cpp) - MIT License
- [PortAudio](http://www.portaudio.com/) - MIT License

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Use Cases

### Desktop Dictation Applications

Perfect for building desktop applications with voice dictation features:
- Note-taking apps
- Voice-controlled interfaces
- Accessibility tools
- Real-time subtitles

### Integration Scenarios

- **C# WPF/WinForms Apps**: Use the provided `RVTTClient.cs` wrapper
- **C# Console Apps**: Simple integration with minimal code
- **Python Apps**: Follow the IPC guide for Python integration
- **Other C++ Apps**: Use the `IPCManager` class directly

## Performance

- **Latency**: ~500ms with base model (depends on model size)
- **CPU Usage**: Moderate (depends on model and CPU)
- **Memory**: ~200-500MB (depends on model size)
- **Transcription Rate**: Real-time (1x audio speed or better)

## Documentation

- **[IPC Integration Guide](docs/IPC_INTEGRATION_GUIDE.md)** - How to integrate with your apps
- **[Usage Guide](docs/USAGE_GUIDE.md)** - Complete usage documentation
- **[Sample App README](csharp/SampleApp/README.md)** - WPF sample guide

## Acknowledgments

- OpenAI for the Whisper speech recognition model
- Georgi Gerganov for whisper.cpp
- PortAudio community for the audio library

