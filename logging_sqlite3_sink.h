#ifndef _LOGGING_SQLITE3_SINK_H_
#define _LOGGING_SQLITE3_SINK_H_
#include "config.h"
#include "sqlite3.h"

#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/frontend_requirements.hpp>

#include <memory>
#include <string>

typedef boost::log::sinks::combine_requirements<
    boost::log::sinks::concurrent_feeding,
    boost::log::sinks::formatted_records,
    boost::log::sinks::flushing>::type frontend_requirements;

struct _logFrame
{
    std::string TimeStamp;
    std::string Severity;
    std::string File;
    std::string Line;
    std::string Function;
    std::string pid;
    std::string tid;
    std::string message;

    std::string __str__()
    {
        std::ostringstream oss;
        oss << "[" << "TimeStamp" << "]" << " " << TimeStamp;
        oss << "[" << "Severity" << "]" << " " << Severity;
        oss << "[" << "File" << "]" << " " << File;
        oss << "[" << "Line" << "]" << " " << Line;
        oss << "[" << "Function" << "]" << " " << Function;
        oss << "[" << "pid" << "]" << " " << pid;
        oss << "[" << "pid" << "]" << " " << pid;
        oss << "[" << "message" << "]" << " " << message;
        return oss.str();
    }
};

class loggingSqlite3Backend : public boost::log::sinks::basic_sink_backend<boost::log::sinks::combine_requirements<boost::log::sinks::concurrent_feeding, boost::log::sinks::flushing>::type>
{

private:
    sqlite3 *db = nullptr;
    int dbOpenStatus;
    std::string dbPath;
    std::string tableName;
    template <typename T>
    std::string extractAttribute(const boost::log::record_view &rec, const char *attributeName);
    void tableInit();
    void closeDB();
    void insertLogFrame(const _logFrame &lf);

public:
    loggingSqlite3Backend(const char *file_name);
    ~loggingSqlite3Backend();

    void consume(boost::log::record_view const &rec);
    void flush();

private:
};
void sqlte3SinkInit(const char *path);
#endif
