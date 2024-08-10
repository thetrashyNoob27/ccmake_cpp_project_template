#ifndef __THREADINFO_H_
#define __THREADINFO_H_

#include <thread>
#include <atomic>
#include <condition_variable>

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
        std::printf("~threadInfo\n");
        if (worker)
        {
            std::printf("worker exist.\n");
            quit = true;
            std::printf("quit true.\n");
            while (status != threadStatus::EXITED)
            {
                cv_worker->notify_all();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                std::printf("thread %zu status%d \n", worker, status.load());
            }
            std::printf("notifyed...\n");
            if (worker->joinable())
            {
                worker->join();
            }
            std::printf("joined.\n");
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