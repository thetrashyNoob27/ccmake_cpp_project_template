#ifndef __DEBUG_PRINTF_H_
#define __DEBUG_PRINTF_H_

#ifdef PIPELINE_DEBUG
#define DEBUG_PRINTF(format, ...) \
    std::printf(format, ##__VA_ARGS__);

#else
#define DEBUG_PRINTF(format, ...) \
    (void)0;
#endif

#endif