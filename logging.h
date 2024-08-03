#ifndef _LOGGING_H_
#define _LOGGING_H_
#include "config.h"

#include <boost/log/utility/setup.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/from_settings.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/from_settings.hpp>

void loggingSetup();
#endif
