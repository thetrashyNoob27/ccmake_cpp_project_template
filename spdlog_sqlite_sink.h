#ifndef _spdlog_sqlite_sink_H_
#define _spdlog_sqlite_sink_H_

#include "spdlog/sinks/sink.h"
#include "sqlite3.h"


namespace spdlog
{
    namespace sinks
    {
        class sqlite_sink : public spdlog::sinks::sink
        {
        public:
            explicit sqlite_sink(const std::string &databaseName)
            {

            }

            ~sqlite_sink()
            {
            }

            virtual void log(const details::log_msg &msg) = 0;
            virtual void flush() = 0;
            virtual void set_pattern(const std::string &pattern) = 0;
            virtual void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) = 0;
        
            void set_level(level::level_enum log_level);
            level::level_enum level() const;
            bool should_log(level::level_enum msg_level) const;
        
        protected:
            level_t level_{level::trace};
        };
    }
}
#endif