#include <iostream>
#include "main.h"

int main(int argc, char **argv, char **env)
{

    std::printf("project: %s version:%s\n", PROJECT_NAME, PROJECT_VERSION);
    print_args(argc, argv);

#ifdef ENABLE_ARG_PARSE
    auto vm = arg_praser(argc, argv);
    if (vm.count("string"))
    {
        std::cout << "string: " << vm["string"].as<std::string>() << std::endl;
    }

    if (vm.count("value"))
    {
        std::cout << "value: " << vm["value"].as<float>() << std::endl;
    }

    if (vm.count("enable"))
    {
        std::cout << "enable: " << vm["enable"].as<bool>() << std::endl;
    }

    if (vm.count("array"))
    {
        std::cout << "Output files:";
        for (const auto &output : vm["array"].as<std::vector<std::string>>())
        {
            std::cout << " " << output;
        }
        std::cout << std::endl;
    }
/* custom content start*/
/* custom content end*/
#else
    std::cout << "boost_program_options no found." << std::endl;
/* custom content start*/
/* custom content end*/
#endif

#ifdef ENABLE_LOGGING
    loggingSetup();
/* custom content start*/
/* custom content end*/
#else
    std::cout << "logging is disabled." << std::endl;
/* custom content start*/
/* custom content end*/
#endif
}
