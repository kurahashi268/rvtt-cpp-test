#pragma once

#include "whisper.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>

class Transcriber {
public:
    Transcriber(const std::string& model_path, const std::string& language = "ja");
    ~Transcriber();

    bool initialize();
    void addAudio(const std::vector<float>& audio_data);
    void start();
    void stop();

    bool isRunning() const { return running_; }

private:
    void processLoop();
    std::string transcribe(const std::vector<float>& audio_data);

    std::string model_path_;
    std::string language_;
    whisper_context* ctx_;
    
    std::atomic<bool> running_;
    std::thread process_thread_;
    
    std::mutex audio_mutex_;
    std::condition_variable audio_cv_;
    std::vector<float> audio_buffer_;
    
    size_t buffer_size_threshold_;
    int sample_rate_;
};

