#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <chrono>
#include <sstream>

namespace core
{
enum class LogLevel { Info, Warning, Error };

// 🔹 ADD: separater Log-Kanal
enum class PipelineLogLevel
{
    Info,
    Error
};

struct LogMessage
{
    LogLevel level;
    std::string text;
    std::string timestamp;
};

class Log
{
public:
    static Log& instance();

    // ---- bestehend ----
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);  

    const std::vector<LogMessage>& messages() const { return m_messages; }
    void enableFileLogging(const std::string& filePath);

    // =====================================================
    // 🔥 ADD: Pipeline / High-Volume Logging
    // =====================================================
    static void enablePipelineLogging(const std::string& filePath);
    static void disablePipelineLogging();
    static void pipelineInfo(const std::string& msg);
    static void pipelineError(const std::string& msg);

private:
    Log() = default;

    // ---- bestehend ----
    void write(LogLevel level, const std::string& msg);
    std::string nowString();

    std::vector<LogMessage> m_messages;
    std::mutex m_mutex;

    std::ofstream m_file;
    bool fileLogging = false;

    // =====================================================
    // 🔥 ADD: Pipeline intern
    // =====================================================
    std::ofstream m_pipelineFile;
    bool pipelineLogging = false;
};
}
