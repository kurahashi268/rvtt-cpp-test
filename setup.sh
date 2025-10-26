#!/bin/bash

set -e

echo "=== Real-time Voice Transcription Tool Setup Script ==="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
fi

echo -e "${BLUE}Detected OS: $OS${NC}"

# Install system dependencies
echo ""
echo "=== Installing System Dependencies ==="

if [ "$OS" = "linux" ]; then
    if command -v apt &> /dev/null; then
        echo "Installing dependencies with apt..."
        sudo apt update
        sudo apt install -y \
            build-essential \
            cmake \
            clang \
            git \
            portaudio19-dev \
            libasound2-dev
    elif command -v pacman &> /dev/null; then
        echo "Installing dependencies with pacman..."
        sudo pacman -S --noconfirm \
            base-devel \
            cmake \
            clang \
            git \
            portaudio
    else
        echo -e "${YELLOW}Unknown package manager. Please install dependencies manually:${NC}"
        echo "  - build-essential/base-devel"
        echo "  - cmake"
        echo "  - clang"
        echo "  - git"
        echo "  - portaudio"
        exit 1
    fi
elif [ "$OS" = "macos" ]; then
    if ! command -v brew &> /dev/null; then
        echo -e "${RED}Homebrew not found. Please install from https://brew.sh${NC}"
        exit 1
    fi
    echo "Installing dependencies with Homebrew..."
    brew install cmake portaudio
else
    echo -e "${RED}Unsupported OS${NC}"
    exit 1
fi

echo -e "${GREEN}✓ System dependencies installed${NC}"

# Check whisper.cpp core
echo ""
echo "=== Checking Whisper.cpp Core ==="

if [ ! -d "core" ]; then
    echo -e "${RED}Error: core/ directory not found${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Whisper.cpp core ready${NC}"

# Download Whisper model
echo ""
echo "=== Downloading Whisper Model ==="

mkdir -p models

if [ -f "models/ggml-base.bin" ]; then
    echo -e "${YELLOW}Model already exists. Skipping download.${NC}"
else
    echo "Downloading base model (suitable for Japanese)..."
    bash core/models/download-ggml-model.sh base
    
    # Move model to models directory if needed
    if [ -f "core/models/ggml-base.bin" ]; then
        mv core/models/ggml-base.bin models/
    fi
fi

echo -e "${GREEN}✓ Model ready${NC}"

# Build the project
echo ""
echo "=== Building Project ==="

chmod +x build.sh
./build.sh

echo ""
echo -e "${GREEN}=== Setup Complete ===${NC}"
echo ""
echo "You can now run the program:"
echo -e "${BLUE}  ./build/rvtt models/ggml-base.bin ja${NC}"
echo ""
echo "For other languages:"
echo "  ./build/rvtt models/ggml-base.bin en  # English"
echo "  ./build/rvtt models/ggml-base.bin zh  # Chinese"
echo "  ./build/rvtt models/ggml-base.bin ko  # Korean"
echo ""

