#ifndef _ARGPROCESSING_H_
#define _ARGPROCESSING_H_
#include "config.h"
#include <cstdlib>
#include <memory>
#include "args.hxx"
#include <filesystem>

class processArgs
{
public:
    static processArgs &getInstance()
    {
        static processArgs instance;
        return instance;
    }
    processArgs(const processArgs &) = delete;
    processArgs &operator=(const processArgs &) = delete;

public:
    std::unique_ptr<args::ArgumentParser> parser;
    std::unique_ptr<args::ValueFlag<std::string>> loggingPath;
    std::unique_ptr<args::ValueFlag<std::string>> dumpProjectSource;

public:
    static void loadArgs(int argc, char **argv)
    {
        getInstance()._loadArgs(argc, argv);
    }

    static std::string GetLoggingPath()
    {

        auto &arg = getInstance().loggingPath;
        if (*arg)
        {
            return args::get(*arg);
        }
        {
            std::filesystem::path tempPath;
            try
            {
                tempPath = std::filesystem::temp_directory_path();
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cout << "accquire temp dir fail:" << e.what();
            }
            return tempPath.string();
        }
    }

private:
    processArgs()
    {
        {
            auto p = new args::ArgumentParser("This is a test program.", "This goes after the options.");
            parser = std::unique_ptr<args::ArgumentParser>(p);
        }

        // log path arg setup
        {
            auto p = new args::ValueFlag<std::string>(*parser, "logging-path", "log output path", {"logging-path"});
            loggingPath = std::unique_ptr<args::ValueFlag<std::string>>(p);
        }
        // dumpProjectSource setup
        {
            auto p = new args::ValueFlag<std::string>(*parser, "dump-project-source", "project tar package save location (save to path)", {"dump-project-source"});
            dumpProjectSource = std::unique_ptr<args::ValueFlag<std::string>>(p);
        }
    }

    void _loadArgs(int argc, char **argv)
    {
        bool parseSuccessful = false;
        try
        {
            parser->ParseCLI(argc, argv);
            parseSuccessful = true;
        }
        catch (args::Help)
        {
            std::cout << *parser;
        }
        catch (args::ParseError e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << *parser;
        }
        catch (args::ValidationError e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << *parser;
        }
        if (parseSuccessful == false)
        {
            std::exit(EXIT_FAILURE);
        }
    }
};

#endif
