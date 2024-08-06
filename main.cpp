#include <iostream>
#include "main.h"

int main(int argc, char **argv, char **env)
{
    auto vm = arg_praser(argc, argv);

    {
        std::string lp = vm["logging-path"].as<std::string>();
        loggingSetup(lp);
    }
    argDebugPrint(vm);
    log_args(argc, argv);
    log_env_vars(env);
    report();
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
}
