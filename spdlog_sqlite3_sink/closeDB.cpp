#include "spdlog_sqlite_sink.h"

namespace spdlog::sinks
{
    void sqlite_sink::closeDB()
    {
        if (!db)
        return;
    sqlite3_close(db);
    db = nullptr;
    }
}