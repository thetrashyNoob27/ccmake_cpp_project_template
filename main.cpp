#include <iostream>
#include "main.h"

int main(int argc, char **argv, char **env)
{
    processArgs::loadArgs(argc, argv);
    // log path setup
    {
        std::string lp = processArgs::GetLoggingPath();
        spdlog_init(lp);
    }
    //log start up message
    {
        std::string loginfo = "\n";
        loginfo += build_info::binaryInfo();
        SPDLOG_INFO(loginfo);
    }

#ifdef ENABLE_PROJECT_ARCHIEVE
    if (vm.count("dump-project-source"))
    {
        auto dumpPath = vm["dump-project-source"].as<std::string>();
        bool saveSuccess;
        std::string errInfo;
        std::cout << "dump project tar enabled. start dump to path: " << dumpPath;
        // SIMPLE_LOGGER(dumpPath, &saveSuccess, &errInfo);
        std::cout << "dump finished(success:" << saveSuccess << ")";
        if (!saveSuccess)
        {
            std::cout << "fail reason:" << errInfo;
        }
        else
        {
            std::cout << "save to:" << errInfo;
        }
        return 0;
    }
#endif
    return 0;
}
