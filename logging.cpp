#include "logging.h"
#include <string>

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
