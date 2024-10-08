#include "logging.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/log/support/date_time.hpp>
#include "build_info.h"
#ifdef ENABLE_PROJECT_ARCHIEVE
#include "project_archieve.h"
#endif
#include <algorithm>
#include <thread>
#include "logging_sqlite3_sink.h"
#include <stdint.h>

boost::log::sources::severity_logger<boost::log::trivial::severity_level> ___GLOBAL_LOGGER___;

static std::string formatNumberWithCommas(int number)
{
    std::string str = std::to_string(number);
    int insertPosition = str.length() - 3;

    while (insertPosition > 0)
    {
        str.insert(insertPosition, ",");
        insertPosition -= 3;
    }

    return str;
}

std::string _basename(const std::string &file_path)
{
    boost::filesystem::path p(file_path);
    return p.filename().string();
}

void loggingSetup(const std::string &loggingBase)
{
    namespace logging = boost::log;
    namespace src = boost::log::sources;
    namespace expr = boost::log::expressions;
    namespace keywords = boost::log::keywords;
    namespace sinks = boost::log::sinks;
    namespace attrs = logging::attributes;

    std::string logTextFileName;
    {
        std::ostringstream oss;
        oss << loggingBase << "/";
        oss << PROJECT_NAME << ".log";
        logTextFileName = oss.str();
    }

    static const std::string COMMON_FMT("[%TimeStamp%]-[%Severity%]-[%File%:%Line%(%Function%)]-[TID:%ThreadID%|PID:%ProcessID%]:  %Message%");
    auto stdoutFormat =
        (expr::stream
         //<< std::hex   //To print the LineID in Hexadecimal format
         << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "]"
         << "[" << logging::trivial::severity << "]"
         << "[" << expr::attr<std::string>("File") << ":" << expr::attr<int>("Line") << "(" << expr::attr<std::string>("Function") << ")" << "]"
         << "[" << "PID:" << expr::attr<logging::attributes::current_process_id::value_type>("ProcessID") << "|" << "TID:" << expr::attr<logging::attributes::current_thread_id::value_type>("ThreadID") << "]"
         << " " << expr::smessage);
    auto textFormat =
        (expr::stream
         << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "]"
         << "[" << logging::trivial::severity << "]"
         << "[" << expr::attr<std::string>("File") << ":" << expr::attr<int>("Line") << "(" << expr::attr<std::string>("Function") << ")" << "]"
         << " " << expr::smessage);

    boost::log::add_common_attributes();
    boost::log::core::get()->add_global_attribute("ThreadID", boost::log::attributes::current_thread_id());
    boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");

    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = textFormat,
        boost::log::keywords::auto_flush = true);

    boost::log::add_file_log(
        boost::log::keywords::file_name = logTextFileName,
        boost::log::keywords::format = stdoutFormat,
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::open_mode = std::ios_base::app);

    BOOST_LOG_TRIVIAL(info) << "project logging setup complete.";


    SIMPLE_LOGGER(info) << "simple logger tester";
}

void report()
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&currentTime);
    // Output the formatted time
    SIMPLE_LOGGER(info) << "project: " << PROJECT_NAME << " " << "version: " << PROJECT_VERSION << " " << "type: " << build_info::buildType;
    SIMPLE_LOGGER(info) << "[git] branch " << build_info::gitBranch << " " << "commit: " << build_info::gitCommit << " " << build_info::gitDirtyStr;
    SIMPLE_LOGGER(info) << "compiler: " << build_info::buildTime << " " << "ID: " << build_info::buildTime;
    SIMPLE_LOGGER(info) << "cmake version: " << build_info::cmakeVersion;
    SIMPLE_LOGGER(info) << "build system: " << build_info::systemName;
    SIMPLE_LOGGER(info) << "binary build time : " << build_info::buildTime;
    SIMPLE_LOGGER(info) << "start time: " << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");

#ifdef ENABLE_PROJECT_ARCHIEVE
    uint8_t *tarData;
    size_t tarDataSize;
    projectSourceTarData(&tarData, &tarDataSize);
    SIMPLE_LOGGER(info) << "souce project tar package size:" << formatNumberWithCommas(tarDataSize) << "Bytes";
#endif
}
