#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class periodicWork
{
public:
    periodicWork(std::function<void()> callback, const unsigned int period = 1000)
        : task(callback), periodMs(period), threadQuit(false)
    {
    }

    virtual ~periodicWork()
    {
        stop();
    }

    void setCallback(std::function<void()> f)
    {
        std::lock_guard<std::mutex> lock(workLock);
        task = std::move(f);
    }

    void start()
    {
        stop();
        threadQuit = false;
        t = new std::thread(&periodicWork::threadJob, this);
    }

    void setPeriod(const unsigned int period)
    {
        std::lock_guard<std::mutex> lock(workLock);
        periodMs = period;
        workCv.notify_one();
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(workLock);
            threadQuit = true;
        }
        workCv.notify_one();
        if (t && t->joinable())
        {
            t->join();
        }
        delete t;
        t = nullptr;
    }

private:
    void threadJob()
    {
        auto lastRun = std::chrono::steady_clock::now();
        while (!threadQuit)
        {
            {
                std::function<void()> localTask;
                {
                    std::lock_guard<std::mutex> lock(workLock);
                    localTask = task;
                }
                if (localTask)
                {
                    localTask();
                }
            }
            {
                std::unique_lock<std::mutex> lock(workLock);
                auto unlockTime = lastRun + std::chrono::milliseconds(periodMs);
                while (!threadQuit && std::chrono::steady_clock::now() < unlockTime)
                {
                    workCv.wait_until(lock, unlockTime);
                    unlockTime = lastRun + std::chrono::milliseconds(periodMs);
                }

                if (threadQuit)
                {
                    break;
                }
                lastRun = unlockTime;
            }
        }
    }

    std::function<void()> task;
    std::thread *t = nullptr;
    std::mutex workLock;
    std::condition_variable workCv;
    unsigned int periodMs;
    std::atomic<bool> threadQuit;
};
