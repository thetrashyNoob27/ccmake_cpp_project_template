# Periodic Work - Thread-based Task Scheduler

Extracted from: src/lcmsControl/modules/clinicalAcquisitionWrapper/modules/topicManageModel/

## Overview

`periodicWork` is a thread-based periodic task executor with configurable period and bounded lag. It separates the **timer** (fixed-rate scheduling) from the **worker** (callback execution) via a `counting_semaphore`, so slow callbacks can lag behind without blocking the timer, and the worker can "chase" buffered ticks up to a configurable limit.

### Key behaviours

| Scenario | Behaviour |
|----------|-----------|
| Callback faster than period | Worker keeps up; ticks execute roughly on schedule. |
| Callback slower than period | Timer gets ahead. Up to `maxLag` ticks are buffered; excess ticks are **dropped**. |
| Worker catches up | If buffered ticks exist, the worker executes them back-to-back (**chase**) without waiting for the next timer deadline. |

## Features

- Configurable period in milliseconds
- **Bounded lag** (`maxLag`) — semaphore-based back-pressure
- Clean start / stop / restart control
- Thread-safe callback updates
- **Multiple callbacks** via `messageDistributor` (async observers)
- Optional **`pipeline` integration** — feed ticks into a processing pipeline
- Automatic cleanup in destructor
- C++17, header-only library

## Dependencies

- C++17 (`std::thread`, `std::atomic`, `std::condition_variable`, `std::mutex`)
- Sibling modules (optional): `messageDistributor`, `pipeline`

## Files Included

| File | Description |
|------|-------------|
| `counting_semaphore.h` | C++17 bounded counting semaphore (lock-free counter, mutex only for blocking) |
| `periodicWork.h` | Main class — timer thread + worker thread + callback dispatch |

## Architecture

```
┌─────────────┐     try_release      ┌─────────────────┐     acquire      ┌─────────────┐
│  timerJob   │ ───────────────────> │ counting_semaphore│ ─────────────> │  workerJob  │
│  (producer) │   (drops if full)    │   maxLag bound    │                │ (consumer)  │
└─────────────┘                      └─────────────────┘                └──────┬──────┘
      │                                                                         │
      │  fixed-rate sleep                                                       │  synchronous primary callback
      │                                                                         │  + async messageDistribute
      ▼                                                                         ▼
   nextRun += period                                                    registered callbacks
```

- **Timer thread** (`timerJob`) maintains a fixed-rate schedule. At each deadline it tries to increment the semaphore; if the semaphore is already at `maxLag` the tick is silently dropped.
- **Worker thread** (`workerJob`) blocks on `acquire()`. When a tick is available it executes the primary callback **synchronously** (so `stop()` can guarantee the callback has finished), then notifies any async observers via `messageDistributor`.
- **`counting_semaphore`** uses a lock-free `std::atomic<std::ptrdiff_t>` counter. The `std::mutex` is used only for `std::condition_variable` blocking/waking — unavoidable in C++17 without `std::atomic::wait` (C++20).

## Quick Deploy Script

Copy and execute in bash to deploy:

```bash
cat << 'EOF' > counting_semaphore.h
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>

class counting_semaphore
{
public:
    explicit counting_semaphore(std::ptrdiff_t max_count)
        : max_count_(max_count), count_(0), stopped_(false) {}

    void acquire()
    {
        while (true)
        {
            auto c = count_.load(std::memory_order_relaxed);
            if (c > 0)
            {
                if (count_.compare_exchange_weak(c, c - 1,
                        std::memory_order_acquire, std::memory_order_relaxed))
                    return;
                continue;
            }
            if (stopped_.load(std::memory_order_acquire))
                return;
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this] {
                return count_.load(std::memory_order_relaxed) > 0 ||
                       stopped_.load(std::memory_order_relaxed);
            });
        }
    }

    bool try_release()
    {
        while (true)
        {
            auto c = count_.load(std::memory_order_relaxed);
            if (stopped_.load(std::memory_order_acquire) || c >= max_count_)
                return false;
            if (count_.compare_exchange_weak(c, c + 1,
                    std::memory_order_release, std::memory_order_relaxed))
            {
                if (c == 0)
                {
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
EOF

cat << 'EOF' > periodicWork.h
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

template <typename Tinput, typename Toutput>
class pipeline;

class periodicWork
{
public:
    periodicWork(std::function<void()> callback = nullptr,
                 const unsigned int period = 1000,
                 std::size_t maxLag = 1)
        : task(callback), periodMs(period), maxLag_(maxLag),
          threadQuit(false), timerThread_(nullptr), workerThread_(nullptr) {}

    virtual ~periodicWork() { stop(); }

    void setCallback(std::function<void()> f)
    {
        std::lock_guard<std::mutex> lock(workLock);
        task = std::move(f);
    }

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
        if (sem_) sem_->stop();
        if (timerThread_ && timerThread_->joinable()) timerThread_->join();
        if (workerThread_ && workerThread_->joinable()) workerThread_->join();
        delete timerThread_; timerThread_ = nullptr;
        delete workerThread_; workerThread_ = nullptr;
        sem_.reset();
    }

    template <typename Tinput, typename Toutput>
    void connectPipeline(pipeline<Tinput, Toutput>* pipe,
                         std::function<Tinput()> factory)
    {
        addCallback([pipe, factory = std::move(factory)]() {
            pipe->addJob(factory());
        });
    }

private:
    void timerJob()
    {
        if (sem_) sem_->try_release();
        auto nextRun = std::chrono::steady_clock::now();
        while (!threadQuit)
        {
            nextRun += std::chrono::milliseconds(periodMs.load(std::memory_order_relaxed));
            std::unique_lock<std::mutex> lock(workLock);
            workCv.wait_until(lock, nextRun, [this] {
                return threadQuit.load() || periodChanged_.load();
            });
            if (threadQuit) break;
            lock.unlock();
            if (periodChanged_)
            {
                periodChanged_ = false;
                nextRun = std::chrono::steady_clock::now();
                continue;
            }
            if (sem_) sem_->try_release();
        }
    }

    void workerJob()
    {
        while (!threadQuit)
        {
            if (!sem_) break;
            sem_->acquire();
            if (threadQuit) break;
            {
                std::function<void()> localTask;
                {
                    std::lock_guard<std::mutex> lock(workLock);
                    localTask = task;
                }
                if (localTask) localTask();
            }
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
EOF

echo "periodicWork.h and counting_semaphore.h created successfully!"
```

## Usage Examples

### Basic Usage

```cpp
#include "periodicWork.h"

// Simple periodic task - runs every 2 seconds
periodicWork scanner([]() {
    std::cout << "Scanning...\n";
}, 2000);

scanner.start();
// ... do other work ...
scanner.stop();
```

### Bounded Lag (Chase Behaviour)

```cpp
#include "periodicWork.h"

// Callback is slow (150ms), period is fast (50ms).
// maxLag=5 lets up to 5 ticks queue; extras are dropped.
// The worker will "chase" by executing back-to-backs after lag.
periodicWork worker([]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    std::cout << "Tick\n";
}, 50, 5);

worker.start();
```

### Multiple Async Observers (messageDistributor)

```cpp
#include "periodicWork.h"

periodicWork worker([]() {
    std::cout << "Primary callback\n";
}, 100);

auto tokenA = worker.addCallback([]() {
    std::cout << "Observer A\n";
});

auto tokenB = worker.addCallback([]() {
    std::cout << "Observer B\n";
});

worker.start();

// Later, remove an observer
worker.removeCallback(tokenA);
```

### Pipeline Integration

```cpp
#include "periodicWork.h"
#include "pipeline.hpp"

class doublePipe : public pipeline<int, int> {
protected:
    int process(const int& x) override { return x * 2; }
};

doublePipe pipe;
pipe.setCallback([](int& x) { std::cout << "Result: " << x << "\n"; });

periodicWork worker(nullptr, 100, 3);
int tick = 1;
worker.connectPipeline(&pipe, std::function<int()>([&]() { return tick++; }));

worker.start();
```

### Dynamic Period

```cpp
#include "periodicWork.h"

periodicWork adaptiveWorker([]() {
    // Work that adapts based on conditions
}, 1000, 3);  // Start with 1 second, maxLag=3

adaptiveWorker.start();

// Later, adjust period based on workload
if (highLoad) {
    adaptiveWorker.setPeriod(500);   // Faster polling
} else {
    adaptiveWorker.setPeriod(5000);  // Slower polling
}
```

## API Reference

### `periodicWork`

| Method | Description |
|--------|-------------|
| `periodicWork(callback, period, maxLag)` | Constructor. `maxLag` defaults to `1` (drop when busy). |
| `start()` | Spin up timer thread + worker thread. Safe to call while already running (calls `stop()` first). |
| `stop()` | Signal quit, wake threads, join both, clean up. |
| `setCallback(callback)` | Replace the primary synchronous callback. |
| `addCallback(callback)` | Add an async observer via `messageDistribute<>`. Returns a token. |
| `removeCallback(token)` | Remove an observer by token. |
| `setPeriod(ms)` | Change the period. Takes effect on the next timer loop iteration. |
| `connectPipeline(pipe, factory)` | Template helper: feed every tick into a `pipeline` instance. |

### `counting_semaphore`

| Method | Description |
|--------|-------------|
| `counting_semaphore(max_count)` | Constructor. |
| `acquire()` | Block until count > 0 or stopped, then decrement. |
| `try_release()` | Increment if below max. Returns `false` if full (tick dropped). |
| `stop()` | Set stopped flag and wake all waiters. |

## Thread Safety Notes

- All public methods of `periodicWork` are thread-safe.
- The primary callback is executed **synchronously** by the worker thread, so `stop()` guarantees it has finished before returning.
- `messageDistribute<>` observers fire in **detached threads**; they do not block the worker thread's chase loop.
- The `std::mutex` inside `counting_semaphore` is used solely for `std::condition_variable` blocking. The counter itself is lock-free (`std::atomic`).
- Destructor automatically stops all threads cleanly.

## Design Trade-offs

| Decision | Rationale |
|----------|-----------|
| Two threads (timer + worker) | Decouples scheduling from execution. A slow callback cannot delay the timer or cause unbounded drift. |
| Semaphore with `maxLag` | Explicit back-pressure. The caller controls how much lag is acceptable; excess ticks are dropped rather than queuing forever. |
| Synchronous primary callback | `stop()` and destructor can guarantee the callback has finished. Async execution is opt-in via `addCallback()`. |
| `messageDistributor` for extras | Reuses the sibling module. Detached-thread dispatch keeps the worker unblocked. |
| C++17 atomics instead of C++20 `std::counting_semaphore` | The project is C++17. The custom implementation is ~50 lines and avoids a `mutex` on the fast path. |
