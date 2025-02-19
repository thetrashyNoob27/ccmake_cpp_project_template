#include "spdlog_sqlite_sink.h"

namespace spdlog::sinks
{
    sqlite_sink::sqlite_sink(const std::string &databaseName)
    {
        loggingSqlite3Backend(databaseName.c_str());
    }
}