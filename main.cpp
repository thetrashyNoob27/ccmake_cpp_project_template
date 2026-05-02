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
    {
        auto &args = processArgs::getInstance();
        if (*args.dumpProjectSource)
        {
            auto dumpPath = args::get(*args.dumpProjectSource);
            std::cout << "dump project tar enabled. start dump to path: " << dumpPath << "\n";
            // TODO: implement project archive dump
            std::cout << "dump finished (not implemented)\n";
            return 0;
        }
    }
#endif
    return 0;
}
