#include <gtest/gtest.h>
#include <periodicWork.h>
#include <atomic>
#include <chrono>
#include <thread>

class PeriodicWorkTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(PeriodicWorkTest, IsRunningReflectsState)
{
    periodicWork worker([]() {}, 100);
    EXPECT_FALSE(worker.isRunning());

    worker.start();
    EXPECT_TRUE(worker.isRunning());

    worker.stop();
    EXPECT_FALSE(worker.isRunning());
}

TEST_F(PeriodicWorkTest, StartIsIdempotent)
{
    std::atomic<int> counter{0};
    periodicWork worker([&counter]() { counter++; }, 50);

    worker.start();
    worker.start(); // should be a no-op
    EXPECT_TRUE(worker.isRunning());

    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    worker.stop();

    EXPECT_GE(counter.load(), 1);
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
    worker.stop();

    EXPECT_FALSE(worker.isRunning());
    EXPECT_TRUE(taskFinished.load())
        << "task should be allowed to finish even when stop() interrupts it";
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
        EXPECT_TRUE(worker.isRunning());
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
    EXPECT_TRUE(worker.isRunning());

    worker.stop();
    EXPECT_FALSE(worker.isRunning());

    worker.stop(); // should not crash
    EXPECT_FALSE(worker.isRunning());
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
