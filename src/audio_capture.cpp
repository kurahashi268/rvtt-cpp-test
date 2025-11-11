#include "audio_capture.h"
#include <iostream>
#include <cstring>

AudioCapture::AudioCapture(int sample_rate, int frames_per_buffer)
    : stream_(nullptr)
    , sample_rate_(sample_rate)
    , frames_per_buffer_(frames_per_buffer)
    , running_(false)
    , pa_initialized_(false) {
}

AudioCapture::~AudioCapture() {
    stop();
    if (stream_) {
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
    if (pa_initialized_) {
        Pa_Terminate();
        pa_initialized_ = false;
    }
}

bool AudioCapture::initialize() {
    if (pa_initialized_) {
        return true;
    }

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    pa_initialized_ = true;

    PaStreamParameters input_parameters;
    input_parameters.device = Pa_GetDefaultInputDevice();
    if (input_parameters.device == paNoDevice) {
        std::cerr << "Error: No default input device." << std::endl;
        Pa_Terminate();
        pa_initialized_ = false;
        return false;
    }

    input_parameters.channelCount = 1; // Mono
    input_parameters.sampleFormat = paFloat32;
    input_parameters.suggestedLatency = 
        Pa_GetDeviceInfo(input_parameters.device)->defaultLowInputLatency;
    input_parameters.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(
        &stream_,
        &input_parameters,
        nullptr, // No output
        sample_rate_,
        frames_per_buffer_,
        paClipOff,
        &AudioCapture::paCallback,
        this
    );

    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        pa_initialized_ = false;
        stream_ = nullptr;
        return false;
    }

    std::cout << "Audio capture initialized at " << sample_rate_ 
              << " Hz, " << frames_per_buffer_ << " frames per buffer" << std::endl;
    
    return true;
}

bool AudioCapture::start(AudioCallback callback) {
    if (running_) {
        return false;
    }

    if (!pa_initialized_ || !stream_) {
        std::cerr << "Audio capture not initialized" << std::endl;
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback_ = callback;
    }

    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    running_ = true;
    std::cout << "Audio capture started" << std::endl;
    return true;
}

void AudioCapture::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    if (stream_) {
        PaError err = Pa_StopStream(stream_);
        if (err != paNoError) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        }
    }

    std::cout << "Audio capture stopped" << std::endl;
}

int AudioCapture::paCallback(
    const void* input_buffer,
    void* output_buffer,
    unsigned long frames_per_buffer,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags status_flags,
    void* user_data
) {
    (void)output_buffer;
    (void)time_info;
    (void)status_flags;

    AudioCapture* capture = static_cast<AudioCapture*>(user_data);
    const float* input = static_cast<const float*>(input_buffer);

    if (input != nullptr) {
        capture->processAudio(input, frames_per_buffer);
    }

    return paContinue;
}

void AudioCapture::processAudio(const float* input, unsigned long frame_count) {
    if (!running_) {
        return;
    }

    std::lock_guard<std::mutex> lock(callback_mutex_);
    
    if (callback_) {
        std::vector<float> audio_data(input, input + frame_count);
        callback_(audio_data);
    }
}

