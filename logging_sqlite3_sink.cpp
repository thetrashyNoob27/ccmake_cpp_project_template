#include "logging.h"
#include "getTypeString.h"
#include "logging_sqlite3_sink.h"
#include "nowTimeString.h"

#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/attr.hpp>
#include "boost/date_time/posix_time/time_formatters_limited.hpp"
#include <boost/log/core.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <iostream>
#include <string>
#include <chrono>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

template <typename T>
std::string loggingSqlite3Backend::extractAttribute(const boost::log::record_view &rec, const char *attributeName)
{
    auto attr_values = rec.attribute_values();
    auto it = attr_values.find(attributeName);
    if (it != attr_values.end())
    {
        auto querryValue = it->second.extract<T>().get();
        std::ostringstream oss;
        oss << querryValue;
        return oss.str();
    }
    return std::string("NULL");
}

void loggingSqlite3Backend::closeDB()
{
    if (!db)
        return;
    sqlite3_close(db);
    db = nullptr;
}

void loggingSqlite3Backend::insertLogFrame(const _logFrame &lf)
{
    if (!db)
    {
        SIMPLE_LOGGER(error) << "sqlite db not open";
        return;
    }
    std::ostringstream oss;
    oss << "INSERT INTO " << "\"" << tableName << "\""
        << "(TimeStamp, Severity, File, Line, Function, pid, tid, message)"
        << "  VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    {
        auto rc = sqlite3_prepare_v2(db, oss.str().c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            SIMPLE_LOGGER(error) << "Failed to prepare statement: " << sqlite3_errmsg(db);
            return;
        }
    }

    {
        int bindIdx = 1;
        sqlite3_bind_text(stmt, bindIdx++, lf.TimeStamp.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, bindIdx++, lf.Severity.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, bindIdx++, lf.File.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, bindIdx++, lf.Line.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, bindIdx++, lf.Function.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, bindIdx++, lf.pid.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, bindIdx++, lf.tid.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, bindIdx++, lf.message.c_str(), -1, SQLITE_STATIC);
    }
    {
        auto rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            SIMPLE_LOGGER(error) << "Failed to insert data: " << sqlite3_errmsg(db);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void loggingSqlite3Backend::tableInit()
{

    tableName = getCurrentTimeString();

    std::ostringstream oss;
    oss << "CREATE TABLE IF NOT EXISTS " << "\"" << tableName << "\"" << " ("
        << "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL" << ","
        << "TimeStamp TEXT" << ","
        << "Severity TEXT" << ","
        << "File TEXT" << ","
        << "Line TEXT" << ","
        << "Function TEXT" << ","
        << "pid TEXT" << ","
        << "tid TEXT" << ","
        << "message TEXT"
        << ");";
    SIMPLE_LOGGER(debug) << "sqlte3 log db table name :" << oss.str() << std::endl;

    char *errMsg = 0;
    auto rc = sqlite3_exec(db, oss.str().c_str(), nullptr, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        SIMPLE_LOGGER(debug) << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    else
    {
        SIMPLE_LOGGER(debug) << "Table" << tableName << " created successfully" << std::endl;
    }
}

loggingSqlite3Backend::loggingSqlite3Backend(const char *file_name)
{
    // init
    dbPath = std::string(file_name) + std::string("/") + PROJECT_NAME + std::string(".sqlite3");
    SIMPLE_LOGGER(debug) << "loggingCsvBackend file:" << dbPath.c_str() << std::endl;

    dbOpenStatus = sqlite3_open(dbPath.c_str(), &db);
    if (dbOpenStatus)
    {
        SIMPLE_LOGGER(error) << "Can't open database: " << sqlite3_errmsg(db);
        closeDB();
    }
    else
    {
        SIMPLE_LOGGER(info) << "open database(success): " << sqlite3_errmsg(db);
    }
    tableInit();
}

loggingSqlite3Backend::~loggingSqlite3Backend()
{
    closeDB();
}

void loggingSqlite3Backend::consume(logging::record_view const &rec)
{
    _logFrame lf;
    lf.TimeStamp = getCurrentTimeString(); // temp solution.
    {
        auto attr_values = rec.attribute_values();
        auto it = attr_values.find("TimeStamp");
        if (it != attr_values.end())
        {
            auto querryValue = it->second.extract<boost::posix_time::ptime>().get();
            lf.TimeStamp = boost::posix_time::to_iso_extended_string(querryValue);
        }
    }
    {
        unsigned int sev = 0;
        if (logging::value_ref<logging::trivial::severity_level> severity =
                rec["Severity"].extract<logging::trivial::severity_level>())
        {
            sev = static_cast<unsigned int>(*severity);
        }
        lf.Severity = std::to_string(sev);
    }

    lf.File = extractAttribute<std::string>(rec, "File");
    lf.Line = extractAttribute<int>(rec, "Line");
    lf.Function = extractAttribute<std::string>(rec, "Function");
    lf.pid = extractAttribute<logging::attributes::current_process_id::value_type>(rec, "pid");
    lf.tid = extractAttribute<logging::attributes::current_thread_id::value_type>(rec, "tid");
    lf.message = *rec[boost::log::expressions::smessage];
    // std::cout << lf.__str__() << std::endl;
    insertLogFrame(lf);
}

void loggingSqlite3Backend::flush()
{
    // called by front end
}

void sqlte3SinkInit(const char *path)
{
    typedef sinks::synchronous_sink<loggingSqlite3Backend> sink_t;

    boost::shared_ptr<logging::core> core = logging::core::get();

    boost::shared_ptr<loggingSqlite3Backend> backend(new loggingSqlite3Backend(path));
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    core->add_sink(sink);

    SIMPLE_LOGGER(info) << "testing";
}