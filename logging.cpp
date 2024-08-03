#include "logging.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <boost/filesystem.hpp>
#include "build_info.h"
#include "project_archieve.h"
#include <algorithm>

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

static std::string _basename(const std::string &file_path)
{
    boost::filesystem::path p(file_path);
    return p.filename().string();
}

void loggingSetup()
{
    namespace logging = boost::log;
    namespace src = boost::log::sources;
    namespace expr = boost::log::expressions;
    namespace keywords = boost::log::keywords;

    std::string logTextFileName;
    {
        std::ostringstream oss;
        oss << PROJECT_NAME << ".log";
        logTextFileName = oss.str();
    }

    static const std::string COMMON_FMT("[%TimeStamp%]-[%Severity%]-[%File%:%LineID%(%Function%)]-[TID:%ThreadID%|PID:%ProcessID%]:  %Message%");

    boost::log::core::get()->add_global_attribute("ThreadID", boost::log::attributes::current_thread_id());
    boost::log::core::get()->add_global_attribute("File", boost::log::attributes::mutable_constant<std::string>(_basename(__FILE__)));
    boost::log::core::get()->add_global_attribute("LineID", boost::log::attributes::mutable_constant<int>(__LINE__));
    boost::log::core::get()->add_global_attribute("Function", boost::log::attributes::mutable_constant<std::string>(__FUNCTION__));

    boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");

    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = COMMON_FMT,
        boost::log::keywords::auto_flush = true);

    boost::log::add_file_log(
        boost::log::keywords::file_name = logTextFileName,
        boost::log::keywords::format = COMMON_FMT,
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::open_mode = std::ios_base::app);

    boost::log::add_common_attributes();

    BOOST_LOG_TRIVIAL(info) << "project logging setup complete.";
}

void report()
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&currentTime);
    // Output the formatted time
    BOOST_LOG_TRIVIAL(info) << "project: " << PROJECT_NAME << " " << "version: " << PROJECT_VERSION << " " << "type: " << build_info::buildType;
    BOOST_LOG_TRIVIAL(info) << "[git] branch " << build_info::gitBranch << " " << "commit: " << build_info::gitCommit << " " << build_info::gitDirtyStr;
    BOOST_LOG_TRIVIAL(info) << "compiler: " << build_info::buildTime << " " << "ID: " << build_info::buildTime;
    BOOST_LOG_TRIVIAL(info) << "cmake version: " << build_info::cmakeVersion;
    BOOST_LOG_TRIVIAL(info) << "build system: " << build_info::systemName;
    BOOST_LOG_TRIVIAL(info) << "binary build time : " << build_info::buildTime;
    BOOST_LOG_TRIVIAL(info) << "start time: " << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");

    uint8_t tarData;
    size_t tarDataSize;
    projectSourceTarData(&tarData, &tarDataSize);
    BOOST_LOG_TRIVIAL(info) << "souce project tar package size:" << formatNumberWithCommas(tarDataSize) << "Bytes";
}
