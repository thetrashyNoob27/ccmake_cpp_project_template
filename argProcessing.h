#ifndef _ARGPROCESSING_H_
#define _ARGPROCESSING_H_

void print_args(int argc, char **argv);
void print_env_vars(char **env);

/* custom content start*/
/* custom content end*/

#include "config.h"
#include "logging.h"
#include <boost/program_options.hpp>
#include <cstdlib>
extern float value;
boost::program_options::variables_map arg_praser(int argc, char **argv);
void argDebugPrint(const boost::program_options::variables_map &vm);
#endif
