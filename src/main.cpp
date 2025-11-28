#include "audio_capture.h"
#include "transcriber.h"
#include "ipc_manager.h"
#include "logger.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>

// Cross-platform signal handling
#ifdef _WIN32
    #include <windows.h>
    #include <signal.h>
#else
    #include <signal.h>
#endif

std::atomic<bool> g_running(true);

#ifdef _WIN32
// Windows console handler
BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
        std::cout << "\nShutting down..." << std::endl;
        g_running = false;
        return TRUE;
    }
    return FALSE;
}
#else
// Unix signal handler
void signalHandler(int signal) {
    (void)signal;
    std::cout << "\nShutting down..." << std::endl;
    g_running = false;
}
#endif

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <model_path> [language] [--test]" << std::endl;
    std::cout << "\nArguments:" << std::endl;
    std::cout << "  model_path: Path to whisper model file (e.g., models/rvtt.bin)" << std::endl;
    std::cout << "  language:   Language code (default: ja for Japanese)" << std::endl;
    std::cout << "  --test:     Run in test mode (CLI transcriber)" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << program_name << " models/rvtt.bin ja --test" << std::endl;
    std::cout << "  " << program_name << " models/rvtt.bin en" << std::endl;
    std::cout << "\nSupported languages: ja, en, zh, ko, etc." << std::endl;
}

// Test mode: CLI audio transcriber
int runTestMode(const std::string& model_path, const std::string& language) {
    std::cout << "=== Real-time Voice Transcription Tool (TEST MODE) ===" << std::endl;
    std::cout << "Model: " << model_path << std::endl;
    std::cout << "Language: " << language << std::endl;
    std::cout << "========================================================" << std::endl;

    // Initialize transcriber
    Transcriber transcriber(model_path, language);
    transcriber.setInitialPrompt("This recording is a medical consultation!", true);
    if (!transcriber.initialize()) {
        std::cerr << "Failed to initialize transcriber" << std::endl;
        return 1;
    }

    // Initialize audio capture
    AudioCapture audio_capture(16000, 512);
    if (!audio_capture.initialize()) {
        std::cerr << "Failed to initialize audio capture" << std::endl;
        Logger::log("Audio capture initialization failed");
        Logger::shutdown();
        return 1;
    }
    Logger::log("Audio capture initialized");

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

// Main mode: Background process for C# integration
int runMainMode(const std::string& model_path, const std::string& language) {
#ifdef _WIN32
    // Disable console output on Windows in main mode
    // (C# application doesn't need to see it)
#endif

    const char* log_override = std::getenv("RVTT_IPC_LOG_FILE");
    std::string log_path = (log_override && *log_override) ? log_override : "rvtt_ipc_debug.log";
    if (!Logger::initialize(log_path)) {
        std::cerr << "Warning: Failed to open IPC log file at " << log_path << std::endl;
    } else {
        Logger::log("Starting IPC main mode. Model=" + model_path + ", Language=" + language);
    }

    // Initialize IPC
    IPCManager ipc_manager("RVTTSharedMemory");
    if (!ipc_manager.initialize()) {
        std::cerr << "Failed to initialize IPC" << std::endl;
        Logger::log("IPC initialization failed");
        Logger::shutdown();
        return 1;
    }
    Logger::log("IPC resources initialized");

    // Initialize transcriber
    Transcriber transcriber(model_path, language);
    transcriber.setConsoleOutput(false); // Disable console output in main mode
    transcriber.setInitialPrompt("This recording is a medical consultation!", true);
    
    if (!transcriber.initialize()) {
        std::cerr << "Failed to initialize transcriber" << std::endl;
        Logger::log("Transcriber initialization failed");
        Logger::shutdown();
        return 1;
    }
    Logger::log("Transcriber initialized");

    // Set up transcription callback to write to shared memory
    transcriber.setTranscriptionCallback([&ipc_manager](const std::string& text, bool is_final) {
        if (!text.empty()) {
            Logger::log("Transcription callback len=" + std::to_string(text.size()) +
                        ", final=" + (is_final ? std::string("true") : std::string("false")));
        }
        if (!ipc_manager.writeTranscription(text, is_final)) {
            Logger::log("writeTranscription skipped or failed");
        }
    });

    // Initialize audio capture
    AudioCapture audio_capture(16000, 512);
    if (!audio_capture.initialize()) {
        std::cerr << "Failed to initialize audio capture" << std::endl;
        return 1;
    }

    // Signal that we're ready
    ipc_manager.signalEvent(IPCEvent::READY);
    Logger::log("Signaled READY event");

    bool listening = false;
    bool transcriber_started = false;
    bool waiting_for_completion = false;
    auto completion_check_start = std::chrono::steady_clock::now();

    // Main event loop
    while (g_running) {
        // Check for quit event (highest priority)
        if (ipc_manager.checkEvent(IPCEvent::QUIT)) {
            Logger::log("Received QUIT event");
            break;
        }

        // Check for stop listen event (process before START_LISTEN to avoid conflicts)
        if (ipc_manager.checkEvent(IPCEvent::STOP_LISTEN)) {
            if (listening) {
                Logger::log("Received STOP_LISTEN event - blocking audio input");
                audio_capture.setInputBlocked(true);
                listening = false;
                // Start checking for completion after stopping
                if (transcriber_started && transcriber.isRunning()) {
                    waiting_for_completion = true;
                    completion_check_start = std::chrono::steady_clock::now();
                    Logger::log("Waiting for pending transcription to finish");
                }
            } else {
                Logger::log("STOP_LISTEN received but already idle");
            }
        }
        
        // Check if transcription is complete (after stopping)
        if (waiting_for_completion && transcriber_started && transcriber.isRunning()) {
            // Check if there's no pending audio and nothing currently processing
            if (!transcriber.hasPendingAudio() && !transcriber.isProcessing()) {
                // Wait a bit more to ensure any in-progress transcription finishes
                auto elapsed = std::chrono::steady_clock::now() - completion_check_start;
                if (elapsed >= std::chrono::milliseconds(500)) {
                    // Signal that transcription is complete
                    ipc_manager.signalEvent(IPCEvent::TRANSCRIPTION_COMPLETE);
                    waiting_for_completion = false;
                    Logger::log("Signaled TRANSCRIPTION_COMPLETE event");
                }
            } else {
                // Reset timer if there's still audio queued or being processed
                completion_check_start = std::chrono::steady_clock::now();
            }
        }

        // Check for start listen event
        if (ipc_manager.checkEvent(IPCEvent::START_LISTEN)) {
            Logger::log("Received START_LISTEN event");
            if (!listening) {
                // Reset completion waiting state when starting again
                waiting_for_completion = false;
                
                // Start transcriber if not already started
                if (!transcriber_started) {
                    transcriber.start();
                    transcriber_started = true;
                    Logger::log("Transcriber thread started");
                }

                if (!audio_capture.isRunning()) {
                    // Start audio capture
                    bool started = audio_capture.start([&transcriber](const std::vector<float>& audio_data) {
                        transcriber.addAudio(audio_data);
                    });

                    if (started) {
                        listening = true;
                        Logger::log("Audio capture started in response to START_LISTEN");
                    } else {
                        Logger::log("Audio capture failed to start after START_LISTEN");
                    }
                } else {
                    audio_capture.setInputBlocked(false);
                    listening = true;
                    Logger::log("Audio capture resumed after START_LISTEN");
                }
            } else {
                Logger::log("START_LISTEN received but already listening");
            }
        }

        // Small sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Cleanup
    if (audio_capture.isRunning()) {
        audio_capture.stop();
        Logger::log("Audio capture stopped");
    }
    
    if (transcriber_started) {
        transcriber.stop();
        Logger::log("Transcriber stopped");
    }

    // Signal that we've terminated
    ipc_manager.signalEvent(IPCEvent::TERMINATED);
    Logger::log("Signaled TERMINATED event");
    
    // Small delay to ensure the event is received
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Logger::log("IPC main mode exiting");
    Logger::shutdown();

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    // Parse arguments
    std::string model_path = argv[1];
    std::string language = "ja";
    bool test_mode = false;

    for (int i = 2; i < argc; ++i) {
        if (std::strcmp(argv[i], "--test") == 0) {
            test_mode = true;
        } else {
            language = argv[i];
        }
    }

    // Set up signal handler (cross-platform)
#ifdef _WIN32
    // Enable UTF-8 output on Windows console
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler(consoleHandler, TRUE);
#else
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
#endif

    // Run in appropriate mode
    if (test_mode) {
        return runTestMode(model_path, language);
    } else {
        return runMainMode(model_path, language);
    }
}
