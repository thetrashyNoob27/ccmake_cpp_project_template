#include "spdlog_init.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/async.h"
#include "config.h"
#include <fstream>
#include <filesystem>
#include "spdlog_sqlite_sink.h"

void spdlog_init(const std::string logFilePath)
{
    std::vector<spdlog::sink_ptr> sinks;

    std::string defualtTextPattern = "[%Y-%m-%d %H:%M:%S.%f][%^%l%$][T:%t|P:%P][%s:%# %!] %v";
    // console sink
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->set_pattern(defualtTextPattern);
        consoleSink->set_formatter(std::move(formatter));
        sinks.push_back(consoleSink);
        spdlog::logger logger("temp", consoleSink);
        logger.info("console log successful setup.");
    }

    // create file sink base path
    bool enableFileSink = false;
    if (logFilePath.size() != 0 && std::filesystem::is_directory(logFilePath))
    {
        std::filesystem::path testlogfile(logFilePath);
        testlogfile = testlogfile / ".testwriteability";
        try
        {
            std::ofstream f(testlogfile, std::ios::app);
            enableFileSink = f.is_open();
            if (enableFileSink)
            {
                std::filesystem::remove(testlogfile);
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            // cant
        }

        enableFileSink = true;
    }

    // text file sink
    if (enableFileSink)
    {
        std::filesystem::path textLogFile(logFilePath);
        {
            std::string fileName = PROJECT_NAME;
            fileName += ".log";
            textLogFile /= fileName;
        }
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(textLogFile.string(), 1024 * 1024 * 10, 3);
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->set_pattern(defualtTextPattern);
        rotating_sink->set_formatter(std::move(formatter));
        sinks.push_back(rotating_sink);

        spdlog::logger logger("temp", rotating_sink);
        logger.info("text log successful setup.");
    }

    // sqlite file sink
    if (enableFileSink)
    {
        std::filesystem::path sqliteLogFile(logFilePath);
        {
            std::string fileName = PROJECT_NAME;
            fileName += ".sqlite3";
            sqliteLogFile /= fileName;
        }

        auto sqlite_sink = std::make_shared<spdlog::sinks::sqlite_sink>(sqliteLogFile.string());
        sinks.push_back(sqlite_sink);

        spdlog::logger logger("temp", sqlite_sink);
        logger.info("sqlite3 log successful setup.");
    }

    spdlog::enable_backtrace(32);
    /*
    use
    spdlog::dump_backtrace();
    to dump backtrace
    */

    // sync logger
    // auto loggers = std::make_shared<spdlog::logger>("global_logger", sinks.begin(), sinks.end());
    // async logger
    spdlog::init_thread_pool(8192, std::thread::hardware_concurrency()); // queue , backing thread.
    auto loggers = std::make_shared<spdlog::async_logger>("global_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::set_default_logger(loggers);
    spdlog::set_level(spdlog::level::debug);

    SPDLOG_INFO("This is a test message");

    {
        std::string str;
        if (enableFileSink)
        {
            str += "file sink for log path:";
            str += logFilePath;
        }
        else
        {
            str += "file path not avaiable, file sink disabled.";
        }
        SPDLOG_INFO(str);
    }
}