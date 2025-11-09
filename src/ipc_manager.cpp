#include "ipc_manager.h"
#include <iostream>
#include <ctime>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
    #include <chrono>
#endif

IPCManager::IPCManager(const std::string& name)
    : name_(name)
    , sequence_number_(0)
#ifdef _WIN32
    , shm_handle_(nullptr)
    , event_ready_(nullptr)
    , event_start_listen_(nullptr)
    , event_stop_listen_(nullptr)
    , event_transcription_complete_(nullptr)
    , event_quit_(nullptr)
    , event_terminated_(nullptr)
    , shm_data_(nullptr)
#else
    , shm_fd_(-1)
    , event_ready_(SEM_FAILED)
    , event_start_listen_(SEM_FAILED)
    , event_stop_listen_(SEM_FAILED)
    , event_transcription_complete_(SEM_FAILED)
    , event_quit_(SEM_FAILED)
    , event_terminated_(SEM_FAILED)
    , shm_data_(nullptr)
#endif
{
}

IPCManager::~IPCManager() {
    cleanup();
}

bool IPCManager::initialize() {
#ifdef _WIN32
    // Windows implementation
    
    // Create shared memory
    shm_handle_ = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        sizeof(TranscriptionData),
        name_.c_str()
    );
    
    if (shm_handle_ == nullptr) {
        std::cerr << "Failed to create shared memory: " << GetLastError() << std::endl;
        return false;
    }
    
    shm_data_ = static_cast<TranscriptionData*>(
        MapViewOfFile(shm_handle_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TranscriptionData))
    );
    
    if (shm_data_ == nullptr) {
        std::cerr << "Failed to map shared memory: " << GetLastError() << std::endl;
        CloseHandle(shm_handle_);
        shm_handle_ = nullptr;
        return false;
    }
    
    // Initialize shared memory (always initialize, even if it already existed)
    memset(shm_data_, 0, sizeof(TranscriptionData));
    
    // Create or open events (CreateEventA opens existing events if they exist)
    event_ready_ = CreateEventA(nullptr, TRUE, FALSE, (name_ + "_Ready").c_str());
    event_start_listen_ = CreateEventA(nullptr, TRUE, FALSE, (name_ + "_StartListen").c_str());
    event_stop_listen_ = CreateEventA(nullptr, TRUE, FALSE, (name_ + "_StopListen").c_str());
    event_transcription_complete_ = CreateEventA(nullptr, TRUE, FALSE, (name_ + "_TranscriptionComplete").c_str());
    event_quit_ = CreateEventA(nullptr, TRUE, FALSE, (name_ + "_Quit").c_str());
    event_terminated_ = CreateEventA(nullptr, TRUE, FALSE, (name_ + "_Terminated").c_str());
    
    if (!event_ready_ || !event_start_listen_ || !event_stop_listen_ || 
        !event_transcription_complete_ || !event_quit_ || !event_terminated_) {
        DWORD error = GetLastError();
        std::cerr << "Failed to create events: " << error << std::endl;
        cleanupPlatformResources();
        return false;
    }
    
    // Reset all events to ensure clean initial state
    // (In case they already existed from a previous run)
    ResetEvent(event_ready_);
    ResetEvent(event_start_listen_);
    ResetEvent(event_stop_listen_);
    ResetEvent(event_transcription_complete_);
    ResetEvent(event_quit_);
    ResetEvent(event_terminated_);
    
    std::cout << "IPC initialized (Windows)" << std::endl;
    return true;
    
#else
    // Linux/POSIX implementation
    
    // Create shared memory
    std::string shm_name = "/" + name_;
    shm_fd_ = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    
    if (shm_fd_ == -1) {
        std::cerr << "Failed to create shared memory: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Set size
    if (ftruncate(shm_fd_, sizeof(TranscriptionData)) == -1) {
        std::cerr << "Failed to set shared memory size: " << strerror(errno) << std::endl;
        close(shm_fd_);
        shm_unlink(shm_name.c_str());
        return false;
    }
    
    // Map shared memory
    shm_data_ = static_cast<TranscriptionData*>(
        mmap(nullptr, sizeof(TranscriptionData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0)
    );
    
    if (shm_data_ == MAP_FAILED) {
        std::cerr << "Failed to map shared memory: " << strerror(errno) << std::endl;
        close(shm_fd_);
        shm_unlink(shm_name.c_str());
        return false;
    }
    
    // Initialize shared memory (always initialize, even if it already existed)
    memset(shm_data_, 0, sizeof(TranscriptionData));
    
    // Create semaphores
    std::string sem_ready = "/" + name_ + "_Ready";
    std::string sem_start = "/" + name_ + "_StartListen";
    std::string sem_stop = "/" + name_ + "_StopListen";
    std::string sem_complete = "/" + name_ + "_TranscriptionComplete";
    std::string sem_quit = "/" + name_ + "_Quit";
    std::string sem_term = "/" + name_ + "_Terminated";
    
    // Unlink old semaphores first (ignore errors if they don't exist)
    sem_unlink(sem_ready.c_str());
    sem_unlink(sem_start.c_str());
    sem_unlink(sem_stop.c_str());
    sem_unlink(sem_complete.c_str());
    sem_unlink(sem_quit.c_str());
    sem_unlink(sem_term.c_str());
    
    // Create new semaphores (all start at 0 = not signaled)
    event_ready_ = sem_open(sem_ready.c_str(), O_CREAT | O_EXCL, 0666, 0);
    event_start_listen_ = sem_open(sem_start.c_str(), O_CREAT | O_EXCL, 0666, 0);
    event_stop_listen_ = sem_open(sem_stop.c_str(), O_CREAT | O_EXCL, 0666, 0);
    event_transcription_complete_ = sem_open(sem_complete.c_str(), O_CREAT | O_EXCL, 0666, 0);
    event_quit_ = sem_open(sem_quit.c_str(), O_CREAT | O_EXCL, 0666, 0);
    event_terminated_ = sem_open(sem_term.c_str(), O_CREAT | O_EXCL, 0666, 0);
    
    if (event_ready_ == SEM_FAILED || event_start_listen_ == SEM_FAILED || 
        event_stop_listen_ == SEM_FAILED || event_transcription_complete_ == SEM_FAILED ||
        event_quit_ == SEM_FAILED || event_terminated_ == SEM_FAILED) {
        std::cerr << "Failed to create semaphores: " << strerror(errno) << std::endl;
        cleanupPlatformResources();
        return false;
    }
    
    std::cout << "IPC initialized (POSIX)" << std::endl;
    return true;
#endif
}

#ifdef _WIN32
HANDLE IPCManager::getEventHandle(IPCEvent event) {
    switch (event) {
        case IPCEvent::READY: return event_ready_;
        case IPCEvent::START_LISTEN: return event_start_listen_;
        case IPCEvent::STOP_LISTEN: return event_stop_listen_;
        case IPCEvent::TRANSCRIPTION_COMPLETE: return event_transcription_complete_;
        case IPCEvent::QUIT: return event_quit_;
        case IPCEvent::TERMINATED: return event_terminated_;
        default: return nullptr;
    }
}
#else
sem_t* IPCManager::getEventSemaphore(IPCEvent event) {
    switch (event) {
        case IPCEvent::READY: return event_ready_;
        case IPCEvent::START_LISTEN: return event_start_listen_;
        case IPCEvent::STOP_LISTEN: return event_stop_listen_;
        case IPCEvent::TRANSCRIPTION_COMPLETE: return event_transcription_complete_;
        case IPCEvent::QUIT: return event_quit_;
        case IPCEvent::TERMINATED: return event_terminated_;
        default: return SEM_FAILED;
    }
}
#endif

bool IPCManager::signalEvent(IPCEvent event) {
#ifdef _WIN32
    HANDLE handle = getEventHandle(event);
    if (handle == nullptr) {
        return false;
    }
    return SetEvent(handle) != 0;
#else
    sem_t* sem = getEventSemaphore(event);
    if (sem == SEM_FAILED) {
        return false;
    }
    return sem_post(sem) == 0;
#endif
}

bool IPCManager::waitForEvent(IPCEvent event, int timeout_ms) {
#ifdef _WIN32
    HANDLE handle = getEventHandle(event);
    if (handle == nullptr) {
        return false;
    }
    
    DWORD wait_time = (timeout_ms < 0) ? INFINITE : static_cast<DWORD>(timeout_ms);
    DWORD result = WaitForSingleObject(handle, wait_time);
    
    if (result == WAIT_OBJECT_0) {
        // Reset the event after receiving it
        ResetEvent(handle);
        return true;
    }
    
    return false;
#else
    sem_t* sem = getEventSemaphore(event);
    if (sem == SEM_FAILED) {
        return false;
    }
    
    if (timeout_ms < 0) {
        // Wait indefinitely
        if (sem_wait(sem) == 0) {
            return true;
        }
    } else {
        // Timed wait
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            return false;
        }
        
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
        
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        if (sem_timedwait(sem, &ts) == 0) {
            return true;
        }
    }
    
    return false;
#endif
}

bool IPCManager::checkEvent(IPCEvent event) {
#ifdef _WIN32
    HANDLE handle = getEventHandle(event);
    if (handle == nullptr) {
        return false;
    }
    
    DWORD result = WaitForSingleObject(handle, 0);
    if (result == WAIT_OBJECT_0) {
        ResetEvent(handle);
        return true;
    }
    return false;
#else
    sem_t* sem = getEventSemaphore(event);
    if (sem == SEM_FAILED) {
        return false;
    }
    
    if (sem_trywait(sem) == 0) {
        return true;
    }
    return false;
#endif
}

bool IPCManager::writeTranscription(const std::string& text, bool is_final) {
    if (shm_data_ == nullptr) {
        return false;
    }
    
    // Skip empty text to avoid unnecessary updates
    if (text.empty()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Copy text (truncate if necessary)
    size_t copy_len = std::min(text.length(), sizeof(shm_data_->text) - 1);
    std::memcpy(shm_data_->text, text.c_str(), copy_len);
    shm_data_->text[copy_len] = '\0';
    
    // Set metadata
#ifdef _WIN32
    auto now = std::chrono::system_clock::now();
    shm_data_->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    shm_data_->timestamp = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
#endif
    
    shm_data_->is_final = is_final;
    shm_data_->sequence_number = ++sequence_number_;
    
    return true;
}

bool IPCManager::readTranscription(TranscriptionData& data) {
    if (shm_data_ == nullptr) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::memcpy(&data, shm_data_, sizeof(TranscriptionData));
    
    return true;
}

void IPCManager::cleanup() {
    cleanupPlatformResources();
}

void IPCManager::cleanupPlatformResources() {
#ifdef _WIN32
    if (shm_data_) {
        UnmapViewOfFile(shm_data_);
        shm_data_ = nullptr;
    }
    if (shm_handle_) {
        CloseHandle(shm_handle_);
        shm_handle_ = nullptr;
    }
    if (event_ready_) {
        CloseHandle(event_ready_);
        event_ready_ = nullptr;
    }
    if (event_start_listen_) {
        CloseHandle(event_start_listen_);
        event_start_listen_ = nullptr;
    }
    if (event_stop_listen_) {
        CloseHandle(event_stop_listen_);
        event_stop_listen_ = nullptr;
    }
    if (event_transcription_complete_) {
        CloseHandle(event_transcription_complete_);
        event_transcription_complete_ = nullptr;
    }
    if (event_quit_) {
        CloseHandle(event_quit_);
        event_quit_ = nullptr;
    }
    if (event_terminated_) {
        CloseHandle(event_terminated_);
        event_terminated_ = nullptr;
    }
#else
    if (shm_data_ && shm_data_ != MAP_FAILED) {
        munmap(shm_data_, sizeof(TranscriptionData));
        shm_data_ = nullptr;
    }
    if (shm_fd_ != -1) {
        close(shm_fd_);
        shm_unlink(("/" + name_).c_str());
        shm_fd_ = -1;
    }
    
    std::string sem_ready = "/" + name_ + "_Ready";
    std::string sem_start = "/" + name_ + "_StartListen";
    std::string sem_stop = "/" + name_ + "_StopListen";
    std::string sem_complete = "/" + name_ + "_TranscriptionComplete";
    std::string sem_quit = "/" + name_ + "_Quit";
    std::string sem_term = "/" + name_ + "_Terminated";
    
    if (event_ready_ != SEM_FAILED) {
        sem_close(event_ready_);
        sem_unlink(sem_ready.c_str());
        event_ready_ = SEM_FAILED;
    }
    if (event_start_listen_ != SEM_FAILED) {
        sem_close(event_start_listen_);
        sem_unlink(sem_start.c_str());
        event_start_listen_ = SEM_FAILED;
    }
    if (event_stop_listen_ != SEM_FAILED) {
        sem_close(event_stop_listen_);
        sem_unlink(sem_stop.c_str());
        event_stop_listen_ = SEM_FAILED;
    }
    if (event_transcription_complete_ != SEM_FAILED) {
        sem_close(event_transcription_complete_);
        sem_unlink(sem_complete.c_str());
        event_transcription_complete_ = SEM_FAILED;
    }
    if (event_quit_ != SEM_FAILED) {
        sem_close(event_quit_);
        sem_unlink(sem_quit.c_str());
        event_quit_ = SEM_FAILED;
    }
    if (event_terminated_ != SEM_FAILED) {
        sem_close(event_terminated_);
        sem_unlink(sem_term.c_str());
        event_terminated_ = SEM_FAILED;
    }
#endif
}

