#include "logging.h"
#include "logging_sqlite3_sink.h"

#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/attr.hpp>

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
BOOST_LOG_ATTRIBUTE_KEYWORD(TID, "ThreadID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)

void loggingCsvBackend::consume(logging::record_view const &rec)
{
    // process log records by rec
    logging::value_ref<unsigned int, tag::line_id> lineID = rec[line_id];
    logging::value_ref<unsigned int, tag::TID> tid = rec[TID];
    std::cout << "line number:" << lineID << " " << "TID: " << tid << std::endl;
}

void loggingCsvBackend::flush()
{
    // called by front end
}

void sinkSetup()
{
    typedef sinks::synchronous_sink<loggingCsvBackend> sink_t;

    boost::shared_ptr<logging::core> core = logging::core::get();

    boost::shared_ptr<loggingCsvBackend> backend(new loggingCsvBackend("stat.csv"));
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    core->add_sink(sink);
}