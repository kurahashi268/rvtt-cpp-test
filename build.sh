#!/bin/bash

set -e

echo "=== Real-time Voice Transcription Tool Build Script ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
echo "Checking for required tools..."

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: cmake not found. Please install cmake.${NC}"
    exit 1
fi

if ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: clang++ not found. Please install clang.${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Required tools found${NC}"

# Check for whisper.cpp core library
echo "Checking whisper.cpp core library..."

if [ ! -d "core" ]; then
    echo -e "${RED}Error: core/ directory not found${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Whisper.cpp ready${NC}"

# Check for PortAudio
echo "Checking for PortAudio..."

if ! pkg-config --exists portaudio-2.0 2>/dev/null && [ ! -f "/usr/include/portaudio.h" ]; then
    echo -e "${RED}Error: PortAudio not found.${NC}"
    echo "Please install PortAudio:"
    echo "  Ubuntu/Debian: sudo apt install portaudio19-dev"
    echo "  Arch Linux: sudo pacman -S portaudio"
    echo "  macOS: brew install portaudio"
    exit 1
fi

echo -e "${GREEN}✓ PortAudio found${NC}"

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building..."
make -j$(nproc)

cd ..

echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Executable location: ./build/rvtt"
echo ""
echo "Next steps:"
echo "1. Download a Whisper model if you haven't already:"
echo "   mkdir -p models"
echo "   bash core/models/download-ggml-model.sh base"
echo ""
echo "2. Run the program:"
echo "   ./build/rvtt models/ggml-base.bin ja"
echo ""

