# Quick Start Guide

## Fast Setup (Automated)

Run the setup script to automatically install dependencies, download models, and build:

```bash
chmod +x setup.sh
./setup.sh
```

Then run:
```bash
./build/rvtt models/ggml-base.bin ja
```

## Manual Setup (Step-by-Step)

### 1. Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install -y build-essential cmake clang git portaudio19-dev libasound2-dev
```

### 2. Verify Whisper.cpp Core

The whisper.cpp library is integrated in the `core/` directory.

```bash
ls -la core/
```

### 3. Download Model

```bash
mkdir -p models
bash core/models/download-ggml-model.sh base
# Move the model file to models/ directory if needed
```

### 4. Build

```bash
./build.sh
```

### 5. Run

```bash
./build/rvtt models/ggml-base.bin ja
```

## Testing Your Setup

### Test Microphone

```bash
# Record 5 seconds of audio
arecord -d 5 -f cd test.wav

# Play it back
aplay test.wav
```

### Test with Different Languages

```bash
# Japanese (default)
./build/rvtt models/ggml-base.bin ja

# English
./build/rvtt models/ggml-base.bin en

# Chinese
./build/rvtt models/ggml-base.bin zh
```

## Common Issues

### "No default input device"
- Check your microphone is connected
- Run `arecord -l` to list devices
- Make sure microphone is not muted in system settings

### "Failed to initialize whisper context"
- Check the model file exists: `ls -lh models/`
- Re-download if needed: `bash core/models/download-ggml-model.sh base`

### Build errors
- Make sure all dependencies are installed
- Try cleaning: `rm -rf build && ./build.sh`

## Performance

**Model comparison for Japanese:**

| Model  | Size  | Speed  | Quality | Recommendation |
|--------|-------|--------|---------|----------------|
| tiny   | 75MB  | Fast   | Basic   | Testing only   |
| base   | 142MB | Fast   | Good    | ✅ Recommended |
| small  | 466MB | Medium | Better  | Good accuracy  |
| medium | 1.5GB | Slow   | Great   | High-end PC    |

## Architecture

```
┌─────────────┐
│ Microphone  │
└──────┬──────┘
       │
       ▼
┌─────────────────┐
│  AudioCapture   │  ← PortAudio (16kHz, mono)
│  (audio_capture.cpp)
└──────┬──────────┘
       │ Audio chunks (512 samples)
       ▼
┌─────────────────┐
│ Audio Buffer    │  ← Accumulates 3s of audio
│ (transcriber.cpp)
└──────┬──────────┘
       │ When buffer full (3s)
       ▼
┌─────────────────┐
│ Whisper.cpp     │  ← Speech recognition
│ (transcriber.cpp)
└──────┬──────────┘
       │ Text output
       ▼
┌─────────────────┐
│ Terminal/stdout │  ← Display transcription
└─────────────────┘
```

## Tips for Better Results

1. **Speak clearly** and at a moderate pace
2. **Reduce background noise** for better accuracy
3. **Use a good microphone** - built-in laptop mics work but external is better
4. **Adjust buffer size** in `src/transcriber.cpp` if you want faster/slower transcription
5. **Try different models** - larger models are more accurate but slower

## What's Next?

- Customize the transcription parameters in `src/transcriber.cpp`
- Add output to file instead of terminal
- Integrate with other applications
- Add punctuation and formatting
- Experiment with different Whisper models

Enjoy your real-time Japanese voice dictation! 🎤✨

