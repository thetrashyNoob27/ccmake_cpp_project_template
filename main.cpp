#include <iostream>
#include "main.h"

int main(int argc, char **argv, char **env)
{

    std::printf("project: %s version:%s\n", PROJECT_NAME, PROJECT_VERSION);
    print_args(argc, argv);

    auto vm = arg_praser(argc, argv);
    argDebugPrint(vm);
    loggingSetup();
}
