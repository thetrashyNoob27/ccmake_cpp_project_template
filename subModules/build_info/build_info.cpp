#include "build_info.h"
#include <iostream>
#include <fstream>
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

    std::string binaryInfo()
    {
        std::string s = "";
        s += "buildTime: ";
        s += buildTime;
        s += "\n";
        s += "compilerName: ";
        s += compilerName;
        s += "\n";
        s += "buildType: ";
        s += buildType;
        s += "\n";
        s += "compilerID: ";
        s += compilerID;
        s += "\n";

        s += "systemName: ";
        s += systemName;
        s += "\n";

        s += "cmakeVersion: ";
        s += cmakeVersion;
        s += "\n";

        s += "gitBranch: ";
        s += gitBranch;
        s += "\n";

        s += "gitCommit: ";
        s += gitCommit;
        s += "\n";

        s += "gitDirtyStr: ";
        s += gitDirtyStr;
        s += "\n";
        return s;
    }

};