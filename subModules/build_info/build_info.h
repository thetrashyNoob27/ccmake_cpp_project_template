#ifndef _BUILD_INFO_H_
#define _BUILD_INFO_H_

#include <string>
#include <cstdint>

namespace build_info
{
    extern const std::string buildTime;
    extern const std::string buildTime;
    extern const std::string compilerName;
    extern const std::string buildType;
    extern const std::string systemName;
    extern const std::string cmakeVersion;
    extern const std::string gitBranch;
    extern const std::string gitCommit;
    extern const std::string gitDirtyStr;
    extern const bool gitDirty;

};

#endif
