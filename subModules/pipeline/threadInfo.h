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
        DEBUG_PRINTF("~threadInfo\n");
        if (worker)
        {
            DEBUG_PRINTF("worker exist.\n");
            quit = true;
            DEBUG_PRINTF("quit true.\n");
            while (status != threadStatus::EXITED)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                cv_worker->notify_all();
                DEBUG_PRINTF("thread %zu status%d \n", worker, status.load());
            }
            DEBUG_PRINTF("notifyed...\n");
            if (worker->joinable())
            {
                worker->join();
            }
            DEBUG_PRINTF("joined.\n");
            delete worker;
        }
        worker = nullptr;
        cv_worker = nullptr;
        status = threadStatus::EXITED;
    }

    std::thread *worker = nullptr;
    std::mutex *cv_lock = nullptr;
    std::condition_variable *cv_worker = nullptr;
    std::atomic<bool> quit = false;
    std::atomic<threadStatus> status = UNKNOW;
};

#endif