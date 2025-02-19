#include "spdlog_sqlite_sink.h"
#include "nowTimeString.h"

namespace spdlog::sinks
{
    void sqlite_sink::tableInit()
    {
        tableName = getCurrentTimeString();

        std::ostringstream oss;
        oss << "CREATE TABLE IF NOT EXISTS " << "\"" << tableName << "\"" << " ("
            << "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL" << ","
            << "timeStamp TEXT" << ","
            << "level TEXT" << ","
            << "file TEXT" << ","
            << "line TEXT" << ","
            << "function TEXT" << ","
            << "pid TEXT" << ","
            << "tid TEXT" << ","
            << "message TEXT"
            << ");";

        char *errMsg = 0;
        auto rc = sqlite3_exec(db, oss.str().c_str(), nullptr, 0, &errMsg);
        if (rc != SQLITE_OK)
        {
            sqlite3_free(errMsg);
        }
        else
        {
        }
    }
}