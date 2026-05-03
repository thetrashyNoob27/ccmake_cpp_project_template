#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "counting_semaphore.h"
#include "messageDistribute.h"

// Forward declaration for optional pipeline integration
template <typename Tinput, typename Toutput>
class pipeline;

class periodicWork
{
public:
    // maxLag controls the semaphore capacity:
    //   1  -> drop ticks when the worker is busy (old behaviour, default)
    //   >1 -> allow up to maxLag ticks to buffer; the worker will
    //         "chase" by executing back-to-back, but never more than
    //         maxLag times in a row.
    periodicWork(std::function<void()> callback = nullptr,
                 const unsigned int period = 1000,
                 std::size_t maxLag = 1)
        : task(callback), periodMs(period), maxLag_(maxLag),
          threadQuit(false), timerThread_(nullptr), workerThread_(nullptr)
    {
    }

    virtual ~periodicWork()
    {
        stop();
    }

    // Replace the primary synchronous callback.
    void setCallback(std::function<void()> f)
    {
        std::lock_guard<std::mutex> lock(workLock);
        task = std::move(f);
    }

    // Add an extra observer via messageDistributor.
    // Observers are fired in detached threads, so they do not block
    // the worker thread.
    auto addCallback(std::function<void()> f)
    {
        return tickObservers_.addUpdateCallback(std::move(f));
    }

    bool removeCallback(const messageDistribute<>::callbackToken& token)
    {
        return tickObservers_.removeUpdateCallback(token);
    }

    void start()
    {
        stop();
        threadQuit = false;
        periodChanged_ = false;
        sem_ = std::make_unique<counting_semaphore>(static_cast<std::ptrdiff_t>(maxLag_));
        timerThread_ = new std::thread(&periodicWork::timerJob, this);
        workerThread_ = new std::thread(&periodicWork::workerJob, this);
    }

    void setPeriod(const unsigned int period)
    {
        periodMs.store(period, std::memory_order_relaxed);
        periodChanged_.store(true, std::memory_order_release);
        workCv.notify_one();
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(workLock);
            threadQuit = true;
        }
        workCv.notify_one();
        if (sem_)
        {
            sem_->stop();
        }
        if (timerThread_ && timerThread_->joinable())
        {
            timerThread_->join();
        }
        if (workerThread_ && workerThread_->joinable())
        {
            workerThread_->join();
        }
        delete timerThread_;
        delete workerThread_;
        timerThread_ = nullptr;
        workerThread_ = nullptr;
        sem_.reset();
    }

    // Optional integration: feed every tick into a pipeline instance.
    // `factory` creates the job object that is passed to pipeline::addJob().
    template <typename Tinput, typename Toutput>
    void connectPipeline(pipeline<Tinput, Toutput>* pipe,
                         std::function<Tinput()> factory)
    {
        addCallback([pipe, factory = std::move(factory)]() {
            pipe->addJob(factory());
        });
    }

private:
    // Timer thread (producer).
    // Maintains a fixed-rate schedule.  When the deadline arrives it
    // tries to release the semaphore; if the semaphore is already full
    // the tick is silently dropped.
    void timerJob()
    {
        // First tick fires immediately to match legacy behaviour.
        if (sem_)
        {
            sem_->try_release();
        }

        auto nextRun = std::chrono::steady_clock::now();
        while (!threadQuit)
        {
            nextRun += std::chrono::milliseconds(periodMs.load(std::memory_order_relaxed));

            std::unique_lock<std::mutex> lock(workLock);
            workCv.wait_until(lock, nextRun, [this] {
                return threadQuit.load() || periodChanged_.load();
            });

            if (threadQuit)
                break;

            lock.unlock();

            if (periodChanged_)
            {
                periodChanged_ = false;
                nextRun = std::chrono::steady_clock::now();
                continue;
            }

            if (sem_)
            {
                sem_->try_release();
            }
        }
    }

    // Worker thread (consumer).
    // Blocks on the semaphore, then executes the callback(s).
    // If the producer has run ahead, the worker will acquire multiple
    // times in quick succession -- "chase" behaviour bounded by maxLag.
    void workerJob()
    {
        while (!threadQuit)
        {
            if (!sem_)
                break;
            sem_->acquire();
            if (threadQuit)
                break;

            // Primary callback -- executed synchronously so that stop()
            // can guarantee the callback has finished before returning.
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

            // Async observers via messageDistributor (detached threads).
            tickObservers_.distribute();
        }
    }

    std::function<void()> task;
    std::atomic<unsigned int> periodMs;
    const std::size_t maxLag_;
    std::atomic<bool> threadQuit;
    std::atomic<bool> periodChanged_{false};

    std::thread* timerThread_;
    std::thread* workerThread_;
    std::unique_ptr<counting_semaphore> sem_;

    std::mutex workLock;
    std::condition_variable workCv;

    messageDistribute<> tickObservers_;
};
