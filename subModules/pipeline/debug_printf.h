#ifndef __DEBUG_PRINTF_H_
#define __DEBUG_PRINTF_H_

#include <thread>
#include <filesystem>
#include <mutex>
#include <unistd.h>

static std::mutex MUTEX_DEBUG_PRINTF;
#ifdef PIPELINE_DEBUG
#define DEBUG_PRINTF(format, ...)                                                                                                                                                \
    MUTEX_DEBUG_PRINTF.lock();                                                                                                                                                   \
    std::printf("[%s:%d(%s)][TID:0x%X|PID:0x%X]-> ", std::filesystem::path(__FILE__).filename().string().c_str(), __LINE__, __FUNCTION__, std::this_thread::get_id(), getpid()); \
    std::printf(format, ##__VA_ARGS__);                                                                                                                                          \
    std::printf("\n");                                                                                                                                                           \
    MUTEX_DEBUG_PRINTF.unlock();
#else
#define DEBUG_PRINTF(format, ...) \
    (void)0;
#endif

#endif