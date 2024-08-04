#include "logging.h"
#include "logging_sqlite3_sink.h"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

loggingCsvBackend::loggingCsvBackend(const char *file_name)
{
    // init
    std::cout << "loggingCsvBackend file:" << file_name << std::endl;
}

void loggingCsvBackend::consume(logging::record_view const &rec)
{
    // process log records by rec
    std::cout << "logged" << std::endl;
}

void loggingCsvBackend::flush()
{
    // called by front end
}
