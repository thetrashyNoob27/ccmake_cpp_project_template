#ifndef _ARGPROCESSING_H_
#define _ARGPROCESSING_H_

void print_args(int argc, char **argv);
void print_env_vars(char **env);

/* custom content start*/
/* custom content end*/

#include "config.h"
#ifdef ENABLE_ARG_PARSE
#include <boost/program_options.hpp>
#include <cstdlib>
extern float value;
boost::program_options::variables_map arg_praser(int argc, char **argv);
/* custom content start*/
/* custom content end*/
#endif
#endif
