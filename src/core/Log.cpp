#include "core/Log.h"
#include <iostream>
#include <iomanip>

namespace core
{
Log& Log::instance()
{
    static Log s;
    return s;
}

// ---- bestehend ----
void Log::enableFileLogging(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_file.open(filePath, std::ios::app);
    fileLogging = m_file.is_open();
    if (fileLogging) info("File logging enabled: " + filePath);
}

void Log::info (const std::string& msg) { instance().write(LogLevel::Info, msg); }
void Log::warn (const std::string& msg) { instance().write(LogLevel::Warning, msg); }
void Log::error(const std::string& msg) { instance().write(LogLevel::Error, msg); }

void Log::write(LogLevel level, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    LogMessage entry { level, msg, nowString() };
    m_messages.push_back(entry);

    const char* p = (level == LogLevel::Warning ? "[WARN]" :
                         level == LogLevel::Error   ? "[ERR ]" : "[INFO]");

    std::cout << entry.timestamp << " " << p << " " << msg << std::endl;

    if (fileLogging)
        m_file << entry.timestamp << " " << p << " " << msg << "\n";
}

// =====================================================
// 🔥 ADD: Pipeline Logging (kein UI, kein Buffer)
// =====================================================

void Log::enablePipelineLogging(const std::string& filePath)
{
    Log& l = instance();

    // bewusst KEIN cout / KEIN info()
    std::lock_guard<std::mutex> lock(l.m_mutex);
    l.m_pipelineFile.open(filePath, std::ios::out | std::ios::trunc);
    l.pipelineLogging = l.m_pipelineFile.is_open();
}

void Log::disablePipelineLogging()
{
    Log& l = instance();
    std::lock_guard<std::mutex> lock(l.m_mutex);

    if (l.m_pipelineFile.is_open())
        l.m_pipelineFile.close();

    l.pipelineLogging = false;
}

void Log::pipelineInfo(const std::string& msg)
{
    Log& l = instance();
    if (!l.pipelineLogging)
        return;

    // kein m_messages, kein stdout
    l.m_pipelineFile
        << l.nowString()
        << " [PIPE] "
        << msg
        << '\n';
}

void Log::pipelineError(const std::string& msg)
{
    Log& l = instance();
    if (!l.pipelineLogging)
        return;

    l.m_pipelineFile
        << l.nowString()
        << " [PIPE][ERR] "
        << msg
        << '\n';
}

std::string Log::nowString()
{
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};
    localtime_s(&tm, &t);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}
}
