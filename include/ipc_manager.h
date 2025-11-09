#pragma once

#include <string>
#include <mutex>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <semaphore.h>
#endif

// Shared memory structure for transcription data
struct TranscriptionData {
    char text[4096];           // Current transcription text
    int64_t timestamp;         // Timestamp of transcription
    bool is_final;             // Whether this is a final result
    int sequence_number;       // Sequence number to track updates
};

// IPC Events for communication
enum class IPCEvent {
    READY,                  // App is ready to receive commands
    START_LISTEN,           // Start listening/transcribing
    STOP_LISTEN,            // Stop listening/transcribing
    TRANSCRIPTION_COMPLETE, // All transcription is complete (no more audio to process)
    QUIT,                   // Quit the application
    TERMINATED              // App has terminated
};

class IPCManager {
public:
    IPCManager(const std::string& name = "RVTTSharedMemory");
    ~IPCManager();

    // Initialize shared memory and events
    bool initialize();
    
    // Event signaling
    bool signalEvent(IPCEvent event);
    bool waitForEvent(IPCEvent event, int timeout_ms = -1); // -1 = infinite
    bool checkEvent(IPCEvent event); // Non-blocking check
    
    // Shared memory operations
    bool writeTranscription(const std::string& text, bool is_final = false);
    bool readTranscription(TranscriptionData& data);
    
    // Cleanup
    void cleanup();

private:
    std::string name_;
    std::mutex mutex_;
    int sequence_number_;

#ifdef _WIN32
    // Windows: Named events and shared memory
    HANDLE shm_handle_;
    HANDLE event_ready_;
    HANDLE event_start_listen_;
    HANDLE event_stop_listen_;
    HANDLE event_transcription_complete_;
    HANDLE event_quit_;
    HANDLE event_terminated_;
    TranscriptionData* shm_data_;
#else
    // Linux: POSIX shared memory and semaphores
    int shm_fd_;
    sem_t* event_ready_;
    sem_t* event_start_listen_;
    sem_t* event_stop_listen_;
    sem_t* event_transcription_complete_;
    sem_t* event_quit_;
    sem_t* event_terminated_;
    TranscriptionData* shm_data_;
#endif

    void cleanupPlatformResources();
    
#ifdef _WIN32
    HANDLE getEventHandle(IPCEvent event);
#else
    sem_t* getEventSemaphore(IPCEvent event);
#endif
};

