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
#include <boost/log/utility/manipulators/add_value.hpp>

std::string _basename(const std::string &file_path);
void loggingSetup(const std::string &loggingBase);
void report();

extern boost::log::sources::severity_logger<boost::log::trivial::severity_level> ___GLOBAL_LOGGER___;

#define SIMPLE_LOGGER(__SV__)                                       \
    BOOST_LOG_SEV(___GLOBAL_LOGGER___, boost::log::trivial::__SV__) \
        << boost::log::add_value("Line", __LINE__)                  \
        << boost::log::add_value("File", _basename(__FILE__))       \
        << boost::log::add_value("Function", BOOST_CURRENT_FUNCTION)
#endif
