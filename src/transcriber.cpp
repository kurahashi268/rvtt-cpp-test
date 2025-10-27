#include "transcriber.h"
#include <iostream>
#include <chrono>

Transcriber::Transcriber(const std::string& model_path, const std::string& language)
    : model_path_(model_path)
    , language_(language)
    , ctx_(nullptr)
    , running_(false)
    , buffer_size_threshold_(16000 * 3) // 3 seconds of audio at 16kHz
    , sample_rate_(16000) {
}

Transcriber::~Transcriber() {
    stop();
    if (ctx_) {
        whisper_free(ctx_);
    }
}

bool Transcriber::initialize() {
    struct whisper_context_params cparams = whisper_context_default_params();
    
    ctx_ = whisper_init_from_file_with_params(model_path_.c_str(), cparams);
    
    if (!ctx_) {
        std::cerr << "Failed to initialize whisper context from: " << model_path_ << std::endl;
        return false;
    }

    std::cout << "Whisper model loaded: " << model_path_ << std::endl;
    std::cout << "Language: " << language_ << std::endl;
    
    return true;
}

void Transcriber::addAudio(const std::vector<float>& audio_data) {
    std::lock_guard<std::mutex> lock(audio_mutex_);
    audio_buffer_.insert(audio_buffer_.end(), audio_data.begin(), audio_data.end());
    
    // Notify processing thread if buffer is large enough
    if (audio_buffer_.size() >= buffer_size_threshold_) {
        audio_cv_.notify_one();
    }
}

void Transcriber::start() {
    if (running_) {
        return;
    }

    running_ = true;
    process_thread_ = std::thread(&Transcriber::processLoop, this);
    
    std::cout << "Transcriber started" << std::endl;
}

void Transcriber::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    audio_cv_.notify_all();
    
    if (process_thread_.joinable()) {
        process_thread_.join();
    }
    
    std::cout << "Transcriber stopped" << std::endl;
}

void Transcriber::processLoop() {
    while (running_) {
        std::vector<float> audio_chunk;
        
        {
            std::unique_lock<std::mutex> lock(audio_mutex_);
            
            // Wait for enough audio data or timeout
            audio_cv_.wait_for(lock, std::chrono::seconds(2), [this] {
                return audio_buffer_.size() >= buffer_size_threshold_ || !running_;
            });
            
            // Exit immediately if stopping - don't process remaining audio
            if (!running_) {
                break;
            }
            
            if (audio_buffer_.size() >= buffer_size_threshold_) {
                audio_chunk = std::move(audio_buffer_);
                audio_buffer_.clear();
            } else {
                continue;
            }
        }
        
        // Double-check we're still running before expensive transcription
        if (!audio_chunk.empty() && running_) {
            std::string text = transcribe(audio_chunk);
            if (!text.empty() && running_) {
                std::cout << text << std::endl;
            }
        }
    }
}

std::string Transcriber::transcribe(const std::vector<float>& audio_data) {
    if (!ctx_ || audio_data.empty()) {
        return "";
    }

    // Prepare whisper parameters
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    
    wparams.print_realtime = false;
    wparams.print_progress = false;
    wparams.print_timestamps = false;
    wparams.print_special = false;
    wparams.translate = false;
    wparams.language = language_.c_str();
    wparams.n_threads = 4;
    wparams.offset_ms = 0;
    wparams.no_context = true;
    wparams.single_segment = false;
    
    // Suppress non-speech tokens
    wparams.suppress_blank = true;
    wparams.suppress_nst = true;

    // Process audio
    int ret = whisper_full(ctx_, wparams, audio_data.data(), audio_data.size());
    
    if (ret != 0) {
        std::cerr << "Failed to process audio" << std::endl;
        return "";
    }

    // Get transcription
    std::string result;
    const int n_segments = whisper_full_n_segments(ctx_);
    
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(ctx_, i);
        if (text) {
            result += text;
        }
    }

    return result;
}

