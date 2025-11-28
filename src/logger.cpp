#include "logger.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace {
std::mutex logger_mutex;
std::ofstream logger_stream;
bool logger_enabled = false;

std::string formatTimestamp(const std::chrono::system_clock::time_point& tp) {
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(tp);

    std::tm tm_snapshot;
#ifdef _WIN32
    localtime_s(&tm_snapshot, &now_c);
#else
    localtime_r(&now_c, &tm_snapshot);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_snapshot, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();
    return oss.str();
}

void writeLocked(const std::string& message) {
    const auto now = std::chrono::system_clock::now();
    logger_stream << formatTimestamp(now) << " | " << message << std::endl;
}
} // namespace

bool Logger::initialize(const std::string& file_path, bool enable) {
    std::lock_guard<std::mutex> lock(logger_mutex);

    if (logger_stream.is_open()) {
        logger_stream.close();
    }

    logger_enabled = enable;

    if (!logger_enabled) {
        return false;
    }

    logger_stream.open(file_path, std::ios::out | std::ios::app);
    if (!logger_stream.is_open()) {
        logger_enabled = false;
        return false;
    }

    writeLocked("=== IPC debug logging started ===");
    return true;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(logger_mutex);
    if (logger_stream.is_open()) {
        writeLocked("=== IPC debug logging stopped ===");
        logger_stream.close();
    }
    logger_enabled = false;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logger_mutex);
    if (!logger_enabled || !logger_stream.is_open()) {
        return;
    }
    writeLocked(message);
}

bool Logger::isEnabled() {
    std::lock_guard<std::mutex> lock(logger_mutex);
    return logger_enabled && logger_stream.is_open();
}


