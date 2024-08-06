#include <iostream>
#include <sstream>
#include "argProcessing.h"
#include "logging.h"
#include <list>

void log_args(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        std::stringstream ss;
        ss << "arg-No." << i << " ";
        ss << "[" << argv[i] << "]";
        SIMPLE_LOGGER(info) << ss.str();
    }
    return;
}

void log_env_vars(char **env)
{
    std::vector<std::string> envList;
    char **envstr = env;
    int env_counter = 0;
    while (*envstr != NULL)
    {
        envList.push_back(std::string(*envstr));
        envstr++;
    }
    for (int i = 0; i < envList.size(); i++)
    {
        std::stringstream ss;
        ss << "env-No." << i << " ";
        ss << "[" << envList[i] << "]";
        SIMPLE_LOGGER(info) << ss.str();
    }
    return;
}

#include <filesystem>
float value;
boost::program_options::variables_map arg_praser(int argc, char **argv)
{
    std::filesystem::path tempPath;
    try
    {
        tempPath = std::filesystem::temp_directory_path();
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        SIMPLE_LOGGER(error) << "accquire temp dir fail:" << e.what();
    }

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message");
    desc.add_options()("string,s", po::value<std::string>(), "string option");
    desc.add_options()("value,v", po::value<float>(&value)->default_value(3.14), "float option");
    desc.add_options()("enable,e", po::bool_switch()->default_value(false), "enable option");
    desc.add_options()("array,a", po::value<std::vector<std::string>>()->multitoken(), "array of options");

    desc.add_options()("logging-path", po::value<std::string>()->default_value(tempPath.string()), "log output location");
#ifdef ENABLE_PROJECT_ARCHIEVE
    desc.add_options()("dump-project-source", po::value<std::string>(), "project tar package save location (save to path)");
#endif

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        std::exit(EXIT_SUCCESS);
    }
    return vm;
}
void argDebugPrint(const boost::program_options::variables_map &vm)
{
#define SINK SIMPLE_LOGGER(info)
    if (vm.count("string"))
    {
        SINK << "arg name->" << "string: " << vm["string"].as<std::string>();
    }

    if (vm.count("value"))
    {
        SINK << "arg name->" << "value: " << vm["value"].as<float>();
    }

    if (vm.count("enable"))
    {
        SINK << "arg name->" << "enable: " << vm["enable"].as<bool>();
    }

    if (vm.count("array"))
    {
        std::ostringstream oss;
        oss << "arg name->" << "Output files:";
        for (const auto &output : vm["array"].as<std::vector<std::string>>())
        {
            oss << " " << output;
        }
        SINK << oss.str();
    }
    if (vm.count("logging-path"))
    {
        SINK << "arg name->" << "enable: " << vm["logging-path"].as<std::string>();
    }
#ifdef ENABLE_PROJECT_ARCHIEVE
    if (vm.count("dump-project-source"))
    {
        SINK << "arg name->" << "enable: " << vm["dump-project-source"].as<std::string>();
    }
#endif
#undef SINK
}