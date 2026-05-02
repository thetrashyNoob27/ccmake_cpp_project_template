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
