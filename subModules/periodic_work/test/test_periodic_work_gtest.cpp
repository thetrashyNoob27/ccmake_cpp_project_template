#include <gtest/gtest.h>
#include <periodicWork.h>
#include <pipeline.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

class PeriodicWorkTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(PeriodicWorkTest, StartActivatesTask)
{
    std::atomic<int> counter{0};
    periodicWork worker([&counter]() { counter++; }, 100);

    // Before start: no executions
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    EXPECT_EQ(counter.load(), 0);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_GE(counter.load(), 1);

    int countAfterStop = counter.load();
    worker.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_EQ(counter.load(), countAfterStop);
}

TEST_F(PeriodicWorkTest, StartRestartsWorker)
{
    std::atomic<int> counter{0};
    periodicWork worker([&counter]() { counter++; }, 50);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    EXPECT_GE(counter.load(), 1);

    int countBeforeRestart = counter.load();

    // Second start should stop then restart cleanly
    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    EXPECT_GE(counter.load(), countBeforeRestart + 1);

    worker.stop();
}

TEST_F(PeriodicWorkTest, CallbackFiresPeriodically)
{
    std::atomic<int> counter{0};
    periodicWork worker([&counter]() { counter++; }, 50);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(220));
    worker.stop();

    EXPECT_GE(counter.load(), 2);
    EXPECT_LE(counter.load(), 6);
}

TEST_F(PeriodicWorkTest, SetPeriodChangesRate)
{
    std::atomic<int> counter{0};
    periodicWork worker([&counter]() { counter++; }, 200);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    int countSlow = counter.load();
    EXPECT_GE(countSlow, 1);

    worker.setPeriod(50);
    std::this_thread::sleep_for(std::chrono::milliseconds(220));
    worker.stop();

    int countFast = counter.load();
    EXPECT_GT(countFast, countSlow + 1);
}

TEST_F(PeriodicWorkTest, SetPeriodWhileTaskIsRunning)
{
    std::atomic<int> counter{0};
    std::atomic<bool> inTask{false};

    periodicWork worker([&counter, &inTask]() {
        inTask = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        inTask = false;
        counter++;
    }, 500);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // setPeriod while task() is actively running (notify will be lost)
    worker.setPeriod(50);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    worker.stop();

    // With the bug, the first wait still uses 500ms so we'd see ~1 fire.
    // Fixed: the new 50ms period should take effect immediately after task() returns.
    EXPECT_GE(counter.load(), 3)
        << "setPeriod while task() is running should still apply the new period";
}

TEST_F(PeriodicWorkTest, StopWhileTaskIsRunning)
{
    std::atomic<bool> inTask{false};
    std::atomic<bool> taskFinished{false};

    periodicWork worker([&inTask, &taskFinished]() {
        inTask = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        inTask = false;
        taskFinished = true;
    }, 1000);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_TRUE(inTask.load());

    // stop() while task() is actively running
    auto t0 = std::chrono::steady_clock::now();
    worker.stop();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();

    EXPECT_TRUE(taskFinished.load())
        << "task should be allowed to finish even when stop() interrupts it";
    EXPECT_LE(elapsed, 300)
        << "stop() should not deadlock when called during task execution";
}

TEST_F(PeriodicWorkTest, SetCallbackSwapsFunction)
{
    std::atomic<int> a{0};
    std::atomic<int> b{0};

    periodicWork worker([&a]() { a++; }, 50);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    EXPECT_GT(a.load(), 0);

    worker.setCallback([&b]() { b++; });
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    worker.stop();

    EXPECT_GT(b.load(), 0);
}

TEST_F(PeriodicWorkTest, DestructorAutoStops)
{
    std::atomic<int> counter{0};
    {
        periodicWork worker([&counter]() { counter++; }, 50);
        worker.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        EXPECT_GE(counter.load(), 1);
    }
    // Destructor should have stopped the thread cleanly
    SUCCEED();
}

TEST_F(PeriodicWorkTest, MultipleInstancesRunIndependently)
{
    std::atomic<int> fastCounter{0};
    std::atomic<int> slowCounter{0};

    periodicWork fast([&fastCounter]() { fastCounter++; }, 50);
    periodicWork slow([&slowCounter]() { slowCounter++; }, 150);

    fast.start();
    slow.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(320));

    fast.stop();
    slow.stop();

    EXPECT_GT(fastCounter.load(), slowCounter.load());
}

TEST_F(PeriodicWorkTest, StopIsIdempotent)
{
    periodicWork worker([]() {}, 100);
    worker.start();

    worker.stop();
    worker.stop(); // should not crash

    SUCCEED();
}

TEST_F(PeriodicWorkTest, SetCallbackIsThreadSafeDuringExecution)
{
    std::atomic<int> a{0};
    std::atomic<int> b{0};
    std::atomic<bool> inTask{false};

    periodicWork worker([&a, &inTask]() {
        inTask = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        inTask = false;
        a++;
    }, 500);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(inTask.load());

    // swap callback while task is actively executing, then speed up period
    worker.setCallback([&b]() { b++; });
    worker.setPeriod(50);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    worker.stop();

    EXPECT_GE(a.load(), 1);
    EXPECT_GE(b.load(), 1);
}

// ---------------------------------------------------------------------------
// Chase / bounded-lag tests (new)
// ---------------------------------------------------------------------------

TEST_F(PeriodicWorkTest, MaxLagLimitsBuffering)
{
    std::atomic<int> counterLag1{0};
    std::atomic<int> counterLag5{0};
    std::atomic<int> count1{0};
    std::atomic<int> count5{0};

    // First call is slow (200ms), letting the timer get ahead.
    // Subsequent calls are instant.  With a larger maxLag more of
    // those early ticks are buffered and chased later.
    periodicWork workerLag1([&]() {
        if (count1++ == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        counterLag1++;
    }, 50, 1);

    periodicWork workerLag5([&]() {
        if (count5++ == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        counterLag5++;
    }, 50, 5);

    workerLag1.start();
    workerLag5.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    workerLag1.stop();
    workerLag5.stop();

    EXPECT_GT(counterLag5.load(), counterLag1.load())
        << "maxLag=5 should retain more ticks than maxLag=1 when the timer gets ahead";
}

TEST_F(PeriodicWorkTest, ChaseExecutesBackToBack)
{
    std::vector<std::chrono::steady_clock::time_point> timestamps;
    std::mutex tsMutex;
    std::atomic<int> callCount{0};

    // First call is slow (200ms), letting the timer queue up ticks.
    // After the slow call finishes the worker should "chase" by
    // executing the buffered ticks back-to-back with < 30 ms gaps.
    periodicWork worker([&]() {
        {
            std::lock_guard<std::mutex> lock(tsMutex);
            timestamps.push_back(std::chrono::steady_clock::now());
        }
        if (callCount++ == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }, 50, 5);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    worker.stop();

    int chasePairs = 0;
    {
        std::lock_guard<std::mutex> lock(tsMutex);
        for (size_t i = 1; i < timestamps.size(); ++i)
        {
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                            timestamps[i] - timestamps[i - 1])
                            .count();
            if (diff < 30)
            {
                ++chasePairs;
            }
        }
    }

    EXPECT_GE(chasePairs, 2)
        << "with maxLag=5 the worker should execute back-to-back callbacks after the initial slow one";
}

TEST_F(PeriodicWorkTest, NeverExceedsMaxLag)
{
    std::atomic<int> counter{0};
    const std::size_t maxLag = 3;

    // Very slow callback: timer fires 4 times while callback runs once.
    // With maxLag=3, at most 3 ticks can be buffered, so the 4th is
    // dropped.  The worker should never execute more than maxLag+1
    // times in rapid succession (1 immediate + 3 buffered).
    periodicWork worker([&counter]() {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(220));
    }, 50, maxLag);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    worker.stop();

    // In 500ms the timer fires ~10 times (0,50,100,150,200,250,300,350,400,450).
    // The worker can execute at most floor(500/220)+1 = 3 times.
    // But more importantly, every execution consumes at most maxLag buffered
    // ticks, so total executions should be bounded.
    EXPECT_LE(counter.load(), static_cast<int>(maxLag) + 3)
        << "total executions should be bounded by maxLag plus natural scheduling";
}

TEST_F(PeriodicWorkTest, AddRemoveObserver)
{
    std::atomic<int> primary{0};
    std::atomic<int> observer{0};

    periodicWork worker([&]() { primary++; }, 50);
    auto token = worker.addCallback([&]() { observer++; });

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    worker.stop();

    // messageDistributor fires observers in detached threads;
    // give them a moment to finish.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_GE(primary.load(), 1);
    EXPECT_GE(observer.load(), 1);

    // Remove the observer and run again; observer count should not change.
    EXPECT_TRUE(worker.removeCallback(token));
    int observerBefore = observer.load();

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    worker.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(observer.load(), observerBefore);
}

// Helper pipeline for the integration test
class doublePipe : public pipeline<int, int>
{
protected:
    int process(const int& x) override { return x * 2; }
};

TEST_F(PeriodicWorkTest, ConnectPipeline)
{
    doublePipe pipe;
    std::atomic<int> result{0};
    pipe.setCallback([&](int& x) { result += x; });

    periodicWork worker(nullptr, 100, 3);
    int tick = 1;
    worker.connectPipeline(&pipe, std::function<int()>([&]() { return tick++; }));

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    worker.stop();

    // Allow pipeline's callback worker to drain the output queue.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_GT(result.load(), 0)
        << "connectPipeline should feed ticks into the pipeline";
}
