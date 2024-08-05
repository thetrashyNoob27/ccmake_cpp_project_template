#include "logging.h"
#include "logging_sqlite3_sink.h"

#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/attr.hpp>

#include <boost/log/core.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <iostream>
#include <string>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

template <typename T>
std::string loggingSqlite3Backend::extractAttribute(const boost::log::record_view &rec, const char *attributeName)
{
    auto attr_values = rec.attribute_values();
    auto it = attr_values.find(attributeName);
    if (it != attr_values.end())
    {
        auto querryValue = it->second.extract<T>().get();
        std::ostringstream oss;
        oss << querryValue;
        return oss.str();
    }
    return std::string("NULL");
}

loggingSqlite3Backend::loggingSqlite3Backend(const char *file_name)
{
    // init
    std::cout << "loggingCsvBackend file:" << file_name << std::endl;
}

void loggingSqlite3Backend::consume(logging::record_view const &rec)
{
    {
        std::string info = extractAttribute<std::string>(rec, "File");
        std::cout << "loggingCsvBackend file:" << info << std::endl;
    }
    {
        std::string info = extractAttribute<int>(rec, "LineID");
        std::cout << "loggingCsvBackend lineNo:" << info << std::endl;
    }
}

void loggingSqlite3Backend::flush()
{
    // called by front end
}

void test_sinkSetup()
{
    typedef sinks::synchronous_sink<loggingSqlite3Backend> sink_t;

    boost::shared_ptr<logging::core> core = logging::core::get();

    boost::shared_ptr<loggingSqlite3Backend> backend(new loggingSqlite3Backend("stat.csv"));
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    core->add_sink(sink);
}