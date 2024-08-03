#ifndef _LOGGING_H_
#define _LOGGING_H_
#include "config.h"

#ifdef ENABLE_LOGGING
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/from_settings.hpp>
#include <boost/log/utility/setup/file.hpp>

void loggingSetup();
/* custom content start*/
/* custom content end*/
#else
/* custom content start*/
/* custom content end*/
#endif

/* custom content start*/
/* custom content end*/
#endif
