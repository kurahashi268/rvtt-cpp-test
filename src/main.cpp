#include "audio_capture.h"
#include "transcriber.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> g_running(true);

void signalHandler(int signal) {
    (void)signal;
    std::cout << "\nShutting down..." << std::endl;
    g_running = false;
}

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <model_path> [language]" << std::endl;
    std::cout << "  model_path: Path to whisper model file (e.g., models/ggml-base.bin)" << std::endl;
    std::cout << "  language:   Language code (default: ja for Japanese)" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << program_name << " models/ggml-base.bin ja" << std::endl;
    std::cout << "\nSupported languages: ja, en, zh, ko, etc." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string model_path = argv[1];
    std::string language = (argc >= 3) ? argv[2] : "ja";

    // Set up signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "=== Real-time Voice Transcription Tool ===" << std::endl;
    std::cout << "Model: " << model_path << std::endl;
    std::cout << "Language: " << language << std::endl;
    std::cout << "==========================================" << std::endl;

    // Initialize transcriber
    Transcriber transcriber(model_path, language);
    if (!transcriber.initialize()) {
        std::cerr << "Failed to initialize transcriber" << std::endl;
        return 1;
    }

    // Initialize audio capture
    AudioCapture audio_capture(16000, 512);
    if (!audio_capture.initialize()) {
        std::cerr << "Failed to initialize audio capture" << std::endl;
        return 1;
    }

    // Start transcription processing
    transcriber.start();

    // Start audio capture with callback
    bool started = audio_capture.start([&transcriber](const std::vector<float>& audio_data) {
        transcriber.addAudio(audio_data);
    });

    if (!started) {
        std::cerr << "Failed to start audio capture" << std::endl;
        return 1;
    }

    std::cout << "\n🎤 Listening... (Press Ctrl+C to stop)" << std::endl;

    // Main loop
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    std::cout << "\nStopping audio capture..." << std::endl;
    audio_capture.stop();
    
    std::cout << "Stopping transcriber..." << std::endl;
    transcriber.stop();

    std::cout << "Goodbye!" << std::endl;
    return 0;
}

