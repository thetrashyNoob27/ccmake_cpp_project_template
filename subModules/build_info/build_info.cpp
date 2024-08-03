#include "build_info.h"
#include <iostream>
using std::string;

namespace build_info
{
    const string buildTimeStr = BUILD_TIME;
    const string compilerName = COMPILER_VERSION;
    const string compilerID = COMPILER_ID;

    void test_debug()
    {
        std::cout << "build_info test" << std::endl;
    }
};