#ifndef _spdlog_sqlite_sink_H_
#define _spdlog_sqlite_sink_H_

#include "spdlog/sinks/sink.h"
#include "sqlite3.h"
#include "logFrame.h"

namespace spdlog::sinks
{
    class sqlite_sink : public spdlog::sinks::sink
    {
    public:
        explicit sqlite_sink(const std::string &databaseName);
        ~sqlite_sink();

        // sqdlog sinks interface
        virtual void log(const details::log_msg &msg) override;
        virtual void flush() override;
        virtual void set_pattern(const std::string &pattern) override;
        virtual void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override;
        void set_level(level::level_enum log_level);
        level::level_enum level() const;
        bool should_log(level::level_enum msg_level) const;

    protected:
        level_t level_{level::trace};

        sqlite3 *db = nullptr;
        int dbOpenStatus;
        std::string dbPath;
        std::string tableName;

        // DB operation methods
        void closeDB();
        void loggingSqlite3Backend(const char *file_name);
        void tableInit();
        void insertLog(const std::unordered_map<std::string, std::string> &msg);
    };
}
#endif