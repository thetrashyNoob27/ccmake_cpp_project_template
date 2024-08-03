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
}
