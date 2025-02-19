#include "spdlog_sqlite_sink.h"
#include <unordered_map>
#include <string>
#include <chrono>
#include <spdlog/fmt/chrono.h>
#include <spdlog/fmt/bundled/core.h>

// get pid
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static pid_t getCurrentPID()
{
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

namespace spdlog::sinks
{

    void sqlite_sink::log(const details::log_msg &msg)
    {
        std::unordered_map<std::string, std::string> messageInfoMap;
        messageInfoMap["timestamp"] = fmt::format("{:%Y-%m-%d %H:%M:%S}", msg.time);
        messageInfoMap["level"] = spdlog::level::to_string_view(msg.level).data();
        messageInfoMap["loggerName"] = msg.logger_name.data();
        messageInfoMap["message"] = msg.logger_name.data();

        messageInfoMap["threadID"] = std::to_string(msg.thread_id);
        messageInfoMap["processID"] = std::to_string(getCurrentPID());
        // file info
        messageInfoMap["file"] = std::string(msg.source.filename);
        messageInfoMap["function"] = std::string(msg.source.funcname);
        messageInfoMap["line"] = std::to_string(msg.source.line);
    }

    void sqlite_sink::flush()
    {
    }

    void sqlite_sink::set_pattern(const std::string &pattern)
    {
    }

    void sqlite_sink::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter)
    {
    }

    void sqlite_sink::set_level(level::level_enum log_level)
    {
        level_ = log_level;
    }

    level::level_enum sqlite_sink::level() const
    {
        auto lvl = static_cast<level::level_enum>(level_.load());
        return lvl;
    }

    bool sqlite_sink::should_log(level::level_enum msg_level) const
    {
        return true;
    }

}