#include "spdlog_sqlite_sink.h"

namespace spdlog::sinks
{
    sqlite_sink::~sqlite_sink()
    {
        closeDB();
    }
}