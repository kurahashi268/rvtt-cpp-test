#pragma once

#include "whisper.h"
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <chrono>

// Forward declaration
class IPCManager;

class Transcriber {
public:
    using TranscriptionCallback = std::function<void(const std::string&, bool)>;

    Transcriber(const std::string& model_path, const std::string& language = "ja");
    ~Transcriber();

    bool initialize();
    void addAudio(const std::vector<float>& audio_data);
    void start();
    void stop();
    
    // Set callback for transcription results (for IPC mode)
    void setTranscriptionCallback(TranscriptionCallback callback);
    
    // Enable/disable console output
    void setConsoleOutput(bool enable) { console_output_ = enable; }

    void setInitialPrompt(const std::string& prompt, bool carry_initial_prompt);

    bool isRunning() const { return running_; }
    
    // Check if there's pending audio to be processed
    bool hasPendingAudio() const;
    bool isProcessing() const { return processing_.load(); }

private:
    void processLoop();
    std::string transcribe(const std::vector<float>& audio_data);
    bool isSilence(const std::vector<float>& audio_data) const;

    std::string model_path_;
    std::string language_;
    whisper_context* ctx_;
    
    std::atomic<bool> running_;
    std::atomic<bool> processing_;
    std::thread process_thread_;
    
    mutable std::mutex audio_mutex_;
    std::condition_variable audio_cv_;
    std::vector<float> audio_buffer_;
    
    size_t buffer_size_threshold_;
    int sample_rate_;
    
    // Callback for transcription results
    TranscriptionCallback transcription_callback_;
    std::mutex callback_mutex_;
    
    // Console output flag
    bool console_output_;
    
    // Silence detection threshold
    double silence_threshold_;
    std::chrono::milliseconds flush_timeout_;
    std::chrono::steady_clock::time_point last_audio_time_;

    std::string initial_prompt_;
    bool carry_initial_prompt_;
};

