#include <iostream>
#include "main.h"

int main(int argc, char **argv, char **env)
{

    log_args(argc, argv);
    log_env_vars(env);

    auto vm = arg_praser(argc, argv);
    argDebugPrint(vm);
    loggingSetup();
    report();
#ifdef ENABLE_PROJECT_ARCHIEVE
    if (vm.count("dump-project-source"))
    {
        auto dumpPath = vm["dump-project-source"].as<std::string>();
        bool saveSuccess;
        std::string errInfo;
        BOOST_LOG_TRIVIAL(info) << "dump project tar enabled. start dump to path: " << dumpPath;
        saveSourceTarData(dumpPath, &saveSuccess, &errInfo);
        BOOST_LOG_TRIVIAL(info) << "dump finished(success:" << saveSuccess << ")";
        if (!saveSuccess)
        {
            BOOST_LOG_TRIVIAL(error) << "fail reason:" << errInfo;
        }
        else
        {
            BOOST_LOG_TRIVIAL(error) << "save to:" << errInfo;
        }
        return 0;
    }
#endif
}
