#ifndef _LOGGING_SQLITE3_SINK_H_
#define _LOGGING_SQLITE3_SINK_H_
#include "config.h"
#include "sqlite3.h"

#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/frontend_requirements.hpp>

typedef boost::log::sinks::combine_requirements<
    boost::log::sinks::concurrent_feeding,
    boost::log::sinks::formatted_records,
    boost::log::sinks::flushing>::type frontend_requirements;

class loggingCsvBackend : public boost::log::sinks::basic_sink_backend<boost::log::sinks::combine_requirements<boost::log::sinks::concurrent_feeding, boost::log::sinks::flushing>::type>
{

private:
public:
    loggingCsvBackend(const char *file_name);

    void consume(boost::log::record_view const &rec);
    void flush();

private:
};

#endif
