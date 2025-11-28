#pragma once

#include <string>

class Logger {
public:
    // Initialize logger with file path. Returns true when logging is enabled.
    static bool initialize(const std::string& file_path, bool enable = true);

    // Close the log file if open.
    static void shutdown();

    // Write a message to the log (no-op if logger disabled).
    static void log(const std::string& message);

    // Returns true when logger is currently enabled and ready.
    static bool isEnabled();
};


