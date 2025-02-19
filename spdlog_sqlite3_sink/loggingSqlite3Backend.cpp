#include "spdlog_sqlite_sink.h"

namespace spdlog::sinks
{
    void sqlite_sink::loggingSqlite3Backend(const char *file_name)
    {
        // init
        dbPath = file_name;

        dbOpenStatus = sqlite3_open(dbPath.c_str(), &db);
        if (dbOpenStatus)
        {
            closeDB();
        }
        else
        {
        }
        tableInit();
    }

}