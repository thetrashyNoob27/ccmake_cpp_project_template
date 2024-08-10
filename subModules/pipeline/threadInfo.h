#ifndef __THREADINFO_H_
#define __THREADINFO_H_

#include <thread>
#include <atomic>
#include <condition_variable>
#include "debug_printf.h"

enum threadStatus
{
    UNKNOW = 0,
    BUSY,
    IDLE,
    EXITED

};

class threadInfo
{
public:
    threadInfo()
    {
    }
    ~threadInfo()
    {
        DEBUG_PRINTF("~threadInfo");
        if (worker)
        {
            DEBUG_PRINTF("(worker:0x%X) start thread quit process.", reinterpret_cast<uintptr_t>(worker));
            int tryCnt = 0;
            while (status != threadStatus::EXITED)
            {
                quit = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                cv_worker->notify_all();
                DEBUG_PRINTF("(worker:0x%X) notify thread... try:%d", reinterpret_cast<uintptr_t>(worker), tryCnt++);
            }
            DEBUG_PRINTF("(worker:0x%X) thread exited.total try:%d", reinterpret_cast<uintptr_t>(worker), tryCnt);
            if (worker->joinable())
            {
                worker->join();
            }
            DEBUG_PRINTF("(worker:0x%X) thread joined.", reinterpret_cast<uintptr_t>(worker));
            delete worker;
        }
        else
        {
            DEBUG_PRINTF("(worker:0x%X) worker thread nullptr", reinterpret_cast<uintptr_t>(worker));
        }
        worker = nullptr;
        cv_worker = nullptr;
        status = threadStatus::EXITED;
    }

    std::thread *worker = nullptr;
    std::mutex *cv_lock = nullptr;
    std::condition_variable *cv_worker = nullptr;
    volatile bool quit = false;
    volatile threadStatus status = UNKNOW;
};

#endif