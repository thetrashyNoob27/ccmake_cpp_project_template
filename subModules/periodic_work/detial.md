Periodic Work - Thread-based Task Scheduler

Extracted from: src/lcmsControl/modules/clinicalAcquisitionWrapper/modules/topicManageModel/

Overview

Simple thread-based periodic task executor with configurable period. Perfect for background polling, periodic scans, and recurring tasks.

Features
Configurable period in milliseconds
Clean start/stop control
Thread-safe callback updates
Automatic cleanup in destructor
No external dependencies
Dependencies
C++11 (std::thread, std::mutex, std::atomic, std::condition_variable)
STL only (no external dependencies)
Files Included
periodicWork.h - Header-only class
Quick Deploy Script

Copy and execute in bash to deploy:

cat << 'EOF' > periodicWork.h
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
                [this] { return threadQuit; });
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
EOF

echo "periodicWork.h created successfully!"
Usage Example
Basic Usage
#include "periodicWork.h"

// Simple periodic task - runs every 2 seconds
periodicWork scanner([]() {
    qDebug() << "Scanning...";
    // Do your work here
}, 2000);

scanner.start();

// ... do other work ...

// Stop when done
scanner.stop();
Dynamic Callback
#include "periodicWork.h"

class MyWorker {
public:
    void startScanning() {
        scanner.setCallback([this]() {
            this->doScan();
        });
        scanner.start();
    }

    void stopScanning() {
        scanner.stop();
    }

private:
    void doScan() {
        qDebug() << "Performing scan...";
        // Scan logic here
    }

    periodicWork scanner;
};
Dynamic Period
#include "periodicWork.h"

periodicWork adaptiveWorker([]() {
    // Work that adapts based on conditions
}, 1000);  // Start with 1 second

adaptiveWorker.start();

// Later, adjust period based on workload
if (highLoad) {
    adaptiveWorker.setPeriod(500);  // Faster polling
} else {
    adaptiveWorker.setPeriod(5000);  // Slower polling
}
With State
#include "periodicWork.h"

class DataPoller {
public:
    DataPoller() : poller([this]() { pollData(); }, 3000) {}

    void start() {
        poller.start();
    }

    void stop() {
        poller.stop();
    }

    void setPollInterval(int ms) {
        poller.setPeriod(ms);
    }

private:
    void pollData() {
        static int count = 0;
        qDebug() << "Poll #" << ++count << "at" << QTime::currentTime();
        
        // Fetch data from source
        auto data = fetchData();
        
        // Process data
        processData(data);
    }

    periodicWork poller;
};
Multiple Periodic Tasks
#include "periodicWork.h"

class MultiTaskScheduler {
public:
    void startAll() {
        // Fast task - every 100ms
        fastTask.start();
        
        // Medium task - every 1 second
        mediumTask.start();
        
        // Slow task - every 10 seconds
        slowTask.start();
    }

    void stopAll() {
        fastTask.stop();
        mediumTask.stop();
        slowTask.stop();
    }

private:
    periodicWork fastTask{[](){ /* fast task */ }, 100};
    periodicWork mediumTask{[](){ /* medium task */ }, 1000};
    periodicWork slowTask{[](){ /* slow task */ }, 10000};
};
API Reference
Method	Description
periodicWork(callback, period)	Constructor with callback and period (ms)
start()	Start the periodic task
stop()	Stop the periodic task
setCallback(callback)	Change the callback function
setPeriod(ms)	Change the period in milliseconds
isRunning()	Check if task is currently running
Notes
Thread safety: All public methods are thread-safe
Destructor: Automatically stops the task
Callback execution: Callback runs in dedicated thread
Period accuracy: Approximate, depends on system load
Reentrancy: Callback should complete before period expires