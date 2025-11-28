#pragma once

#include <portaudio.h>
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>

class AudioCapture {
public:
    using AudioCallback = std::function<void(const std::vector<float>&)>;

    AudioCapture(int sample_rate = 16000, int frames_per_buffer = 512);
    ~AudioCapture();

    bool initialize();
    bool start(AudioCallback callback);
    void stop();
    void setInputBlocked(bool blocked);
    bool isInputBlocked() const { return input_blocked_.load(); }
    bool isRunning() const { return running_; }

    int getSampleRate() const { return sample_rate_; }

private:
    static int paCallback(
        const void* input_buffer,
        void* output_buffer,
        unsigned long frames_per_buffer,
        const PaStreamCallbackTimeInfo* time_info,
        PaStreamCallbackFlags status_flags,
        void* user_data
    );

    void processAudio(const float* input, unsigned long frame_count);

    PaStream* stream_;
    int sample_rate_;
    int frames_per_buffer_;
    std::atomic<bool> running_;
    std::atomic<bool> input_blocked_;
    AudioCallback callback_;
    std::mutex callback_mutex_;
    bool pa_initialized_;
};

