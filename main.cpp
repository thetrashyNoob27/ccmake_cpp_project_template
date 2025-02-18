#include <iostream>
#include "main.h"

int main(int argc, char **argv, char **env)
{
    processArgs::loadArgs(argc,argv);
    // log path setup
    {
        std::string lp =processArgs::GetLoggingPath();
        loggingSetup(lp);
        if(lp.size()!=0)
        {
            sqlte3SinkInit(lp.c_str());
        }
    }

#ifdef ENABLE_PROJECT_ARCHIEVE
    if (vm.count("dump-project-source"))
    {
        auto dumpPath = vm["dump-project-source"].as<std::string>();
        bool saveSuccess;
        std::string errInfo;
        SIMPLE_LOGGER(info) << "dump project tar enabled. start dump to path: " << dumpPath;
        // SIMPLE_LOGGER(dumpPath, &saveSuccess, &errInfo);
        SIMPLE_LOGGER(info) << "dump finished(success:" << saveSuccess << ")";
        if (!saveSuccess)
        {
            SIMPLE_LOGGER(error) << "fail reason:" << errInfo;
        }
        else
        {
            SIMPLE_LOGGER(error) << "save to:" << errInfo;
        }
        return 0;
    }
#endif
return 0;
}
