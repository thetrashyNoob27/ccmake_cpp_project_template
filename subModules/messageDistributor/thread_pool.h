#ifndef THREAD_POOL_H
#define THREAD_POOL_H

// Based on the classic progschj ThreadPool (https://github.com/progschj/ThreadPool)
// Adapted with a fire-and-forget enqueue() for zero-overhead task submission.

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class thread_pool
{
public:
    explicit thread_pool(std::size_t threads = 2)
        : stop_(false)
    {
        if (threads == 0) threads = 1;
        for (std::size_t i = 0; i < threads; ++i)
        {
            workers_.emplace_back([this] {
                for (;;)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty())
                            return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    // Fire-and-forget: zero overhead, no std::future allocation.
    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_)
                return;
            tasks_.emplace(std::move(task));
        }
        condition_.notify_one();
    }

    // Future-based: use when you need the return value.
    template <class F, class... Args>
    auto enqueue_future(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_)
                throw std::runtime_error("enqueue on stopped thread_pool");
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return res;
    }

    std::size_t pending_tasks() const
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

    ~thread_pool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_)
            worker.join();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

#endif
