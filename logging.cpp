#include "logging.h"

#ifdef ENABLE_LOGGING

void loggingSetup()
{
    boost::log::add_file_log("sample.log");

    BOOST_LOG_TRIVIAL(info) << "This is an informational message";
    BOOST_LOG_TRIVIAL(warning) << "This is a warning message";
    BOOST_LOG_TRIVIAL(error) << "This is an error message";
}
/* custom content start*/
/* custom content end*/
#else
/* custom content start*/
/* custom content end*/
#endif