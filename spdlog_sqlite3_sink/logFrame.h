#ifndef _LOGFRAME_H_
#define _LOGFRAME_H_
#define _spdlog_sqlite_sink_H_
#include <sstream> 
#include <string>

struct logFrame
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
        oss << "[" << "tid" << "]" << " " << tid;
        oss << "[" << "message" << "]" << " " << message;
        return oss.str();
    }
};


#endif