#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>

// C++17 counting semaphore.
// The counter is lock-free (std::atomic); the mutex is used only to
// block / wake threads via the condition variable.
class counting_semaphore
{
public:
    explicit counting_semaphore(std::ptrdiff_t max_count)
        : max_count_(max_count), count_(0), stopped_(false)
    {
    }

    void acquire()
    {
        while (true)
        {
            // Fast path: lock-free CAS
            auto c = count_.load(std::memory_order_relaxed);
            if (c > 0)
            {
                if (count_.compare_exchange_weak(c, c - 1,
                        std::memory_order_acquire, std::memory_order_relaxed))
                {
                    return;
                }
                continue;
            }
            if (stopped_.load(std::memory_order_acquire))
            {
                return;
            }
            // Slow path: block until a tick arrives or we are stopped
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this] {
                return count_.load(std::memory_order_relaxed) > 0 ||
                       stopped_.load(std::memory_order_relaxed);
            });
        }
    }

    // Release if below max. Returns false if already full (tick dropped).
    bool try_release()
    {
        while (true)
        {
            auto c = count_.load(std::memory_order_relaxed);
            if (stopped_.load(std::memory_order_acquire) || c >= max_count_)
            {
                return false;
            }
            if (count_.compare_exchange_weak(c, c + 1,
                    std::memory_order_release, std::memory_order_relaxed))
            {
                if (c == 0)
                {
                    // Wake a waiter only if we transitioned 0 -> 1
                    std::lock_guard<std::mutex> lock(mtx_);
                    cv_.notify_one();
                }
                return true;
            }
        }
    }

    void stop()
    {
        stopped_.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> lock(mtx_);
        cv_.notify_all();
    }

private:
    const std::ptrdiff_t max_count_;
    std::atomic<std::ptrdiff_t> count_;
    std::atomic<bool> stopped_;
    std::mutex mtx_;
    std::condition_variable cv_;
};
