#include "spdlog_sqlite_sink.h"
#include <unordered_map>
#include <vector>
#include <iostream>

namespace spdlog::sinks
{
    void sqlite_sink::insertLog(const std::unordered_map<std::string, std::string> &msg)
    {
        if (!db)
        {
            return;
        }
        std::ostringstream oss;
        oss << "INSERT INTO " << "\"" << tableName << "\""
            << "(timeStamp, level, file, line, function, pid, tid, message)"
            << "  VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

        sqlite3_stmt *stmt;
        {
            auto rc = sqlite3_prepare_v2(db, oss.str().c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK)
            {
                std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
                return;
            }
        }
        const std::string _blankStr = "";
        {
            int bindIdx = 1;
            auto bindInfo = [&](const std::string &name)
            {
                const char *text = _blankStr.c_str();
                auto it = msg.find(name);
                if (it != msg.end())
                {
                    text = it->second.c_str();
                }
                sqlite3_bind_text(stmt, bindIdx++, text, -1, SQLITE_STATIC);
            };
            const std::vector<std::string> infoName = {"timestamp", "level", "file", "line", "function", "processID", "threadID", "message"};
            for (const auto &key : infoName)
            {
                bindInfo(key);
            }
        }
        {
            auto rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                std::cout << "Failed to insert data: " << sqlite3_errmsg(db) << std::endl;
            }
        }
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }
}