#ifndef _ARGPROCESSING_H_
#define _ARGPROCESSING_H_
#include "config.h"
#include "logging.h"
#include <boost/program_options.hpp>
#include <cstdlib>

void log_args(int argc, char **argv);
void log_env_vars(char **env);

extern float value;
boost::program_options::variables_map arg_praser(int argc, char **argv);
void argDebugPrint(const boost::program_options::variables_map &vm);
#endif
