#include "build_info.h"
#include <iostream>
#include <fstream>
#include "root-project-tar.h"
using std::string;

namespace build_info
{
    const string buildTime = BUILD_TIME;
    const string compilerName = COMPILER_VERSION;
    const string buildType = BUILD_TYPE;
    const string compilerID = COMPILER_ID;
    const string systemName = SYSTEM_NAME;
    const string cmakeVersion = CMAKE_SYSTEM_VERSION;
    const string gitBranch = string(GIT_BRANCH);
    const string gitCommit = string(GIT_COMMIT_HASH);
    const string gitDirtyStr = string(GIT_DIRTY);
    const bool gitDirty = gitDirtyStr == string("DIRTY");

    void projectSourceTarData(uint8_t *tarData, uintptr_t *size)
    {
        tarData = test_h;
        *size = reinterpret_cast<uintptr_t>(&test_h_end) - reinterpret_cast<uintptr_t>(test_h);
    }
};