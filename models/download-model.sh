#!/bin/bash

# Script to download a specific Whisper GGML model
# Usage: ./download-model.sh [model_name]
# Default model: base

# Color codes for output
BOLD="\033[1m"
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
RESET="\033[0m"

# Hugging Face repository
SRC="https://huggingface.co/ggerganov/whisper.cpp"
PFX="resolve/main/ggml"

# Available models
MODELS=(
    "tiny" "tiny.en" "tiny-q5_1" "tiny.en-q5_1" "tiny-q8_0"
    "base" "base.en" "base-q5_1" "base.en-q5_1" "base-q8_0"
    "small" "small.en" "small-q5_1" "small.en-q5_1" "small-q8_0"
    "medium" "medium.en" "medium-q5_0" "medium.en-q5_0" "medium-q8_0"
    "large-v1" "large-v2" "large-v2-q5_0" "large-v2-q8_0"
    "large-v3" "large-v3-q5_0"
    "large-v3-turbo" "large-v3-turbo-q5_0" "large-v3-turbo-q8_0"
)

# Function to list available models
list_models() {
    echo -e "\n${BOLD}Available models:${RESET}"
    echo "  Tiny models:    tiny, tiny.en, tiny-q5_1, tiny.en-q5_1, tiny-q8_0"
    echo "  Base models:    base, base.en, base-q5_1, base.en-q5_1, base-q8_0"
    echo "  Small models:   small, small.en, small-q5_1, small.en-q5_1, small-q8_0"
    echo "  Medium models:  medium, medium.en, medium-q5_0, medium.en-q5_0, medium-q8_0"
    echo "  Large models:   large-v1, large-v2, large-v2-q5_0, large-v2-q8_0"
    echo "                  large-v3, large-v3-q5_0"
    echo "  Turbo models:   large-v3-turbo, large-v3-turbo-q5_0, large-v3-turbo-q8_0"
    echo -e "\n${YELLOW}Note:${RESET}"
    echo "  .en = English-only models (faster, smaller)"
    echo "  -q5_0/-q5_1/-q8_0 = Quantized models (smaller file size)"
    echo ""
}

# Function to check if model is valid
is_valid_model() {
    local model=$1
    for m in "${MODELS[@]}"; do
        if [ "$m" == "$model" ]; then
            return 0
        fi
    done
    return 1
}

# Get model name from argument or use default
MODEL="${1:-base}"

# Display usage if --help or -h is passed
if [ "$MODEL" == "--help" ] || [ "$MODEL" == "-h" ]; then
    echo "Usage: $0 [model_name]"
    echo ""
    echo "Download a Whisper GGML model from Hugging Face."
    echo "If no model name is specified, 'base' will be downloaded."
    list_models
    exit 0
fi

# Validate model
if ! is_valid_model "$MODEL"; then
    echo -e "${RED}Error: Invalid model '${MODEL}'${RESET}"
    list_models
    exit 1
fi

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR" || exit 1

# Check if model already exists
if [ -f "ggml-${MODEL}.bin" ]; then
    echo -e "${YELLOW}Model 'ggml-${MODEL}.bin' already exists. Skipping download.${RESET}"
    exit 0
fi

# Download the model
echo -e "${GREEN}Downloading ggml model '${MODEL}' from Hugging Face...${RESET}"
echo "Source: ${SRC}/${PFX}-${MODEL}.bin"
echo "Destination: ${SCRIPT_DIR}/ggml-${MODEL}.bin"
echo ""

# Use wget, curl, or wget2 depending on what's available
if command -v wget2 >/dev/null 2>&1; then
    wget2 --no-config --progress bar -O "ggml-${MODEL}.bin" "${SRC}/${PFX}-${MODEL}.bin"
elif command -v curl >/dev/null 2>&1; then
    curl -L --progress-bar -o "ggml-${MODEL}.bin" "${SRC}/${PFX}-${MODEL}.bin"
elif command -v wget >/dev/null 2>&1; then
    wget --no-config --quiet --show-progress -O "ggml-${MODEL}.bin" "${SRC}/${PFX}-${MODEL}.bin"
else
    echo -e "${RED}Error: wget, curl, or wget2 is required to download models.${RESET}"
    echo "Please install one of these tools and try again."
    exit 1
fi

# Check if download was successful
if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}${BOLD}Success!${RESET} Model downloaded to: ${SCRIPT_DIR}/ggml-${MODEL}.bin"
    
    # Display file size
    if command -v du >/dev/null 2>&1; then
        SIZE=$(du -h "ggml-${MODEL}.bin" | cut -f1)
        echo "File size: ${SIZE}"
    fi
    
    echo -e "\n${BOLD}Usage example:${RESET}"
    echo "  ./rvtt -m models/ggml-${MODEL}.bin"
    echo ""
else
    echo -e "\n${RED}${BOLD}Error:${RESET} Failed to download model '${MODEL}'"
    echo "Please check your internet connection and try again."
    exit 1
fi

