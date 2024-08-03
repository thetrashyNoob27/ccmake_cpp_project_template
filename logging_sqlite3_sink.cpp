#include "logging.h"
#include "logging_sqlite3_sink.h"

#include <boost/log/sinks/async_frontend.hpp>

// Related headers
#include <boost/log/sinks/unbounded_fifo_queue.hpp>
#include <boost/log/sinks/unbounded_ordering_queue.hpp>
#include <boost/log/sinks/bounded_fifo_queue.hpp>
#include <boost/log/sinks/bounded_ordering_queue.hpp>
#include <boost/log/sinks/drop_on_overflow.hpp>
#include <boost/log/sinks/block_on_overflow.hpp>
/*
readme :https://www.boost.org/doc/libs/1_85_0/libs/log/doc/html/log/detailed/sink_frontends.html#log.detailed.sink_frontends.async
*/
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
