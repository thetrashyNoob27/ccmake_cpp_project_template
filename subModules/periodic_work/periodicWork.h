#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class periodicWork
{
public:
    periodicWork(std::function<void()> callback, const unsigned int period = 1000)
        : task(callback), periodMs(period), threadQuit(false)
    {}

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
        if (threadRunning) return;

        threadQuit = false;
        threadRunning = true;
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
        if (!threadRunning) return;

        threadQuit = true;
        workCv.notify_one();
        if (t && t->joinable()) {
            t->join();
        }
        delete t;
        t = nullptr;
        threadRunning = false;
    }

    bool isRunning() const
    {
        return threadRunning;
    }

private:
    void threadJob()
    {
        while (!threadQuit) {
            task();

            std::unique_lock<std::mutex> lock(workLock);
            workCv.wait_for(lock, std::chrono::milliseconds(periodMs),
                [this] { return threadQuit.load(); });
        }
    }

    std::function<void()> task;
    std::thread *t = nullptr;
    std::mutex workLock;
    std::condition_variable workCv;
    unsigned int periodMs;
    std::atomic<bool> threadQuit;
    std::atomic<bool> threadRunning = false;
};
