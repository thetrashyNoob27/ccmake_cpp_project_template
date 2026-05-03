#include <gtest/gtest.h>
#include <messageDistribute.h>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

static void yield_for_async()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

class MessageDistributeTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(MessageDistributeTest, AddAndDistributeSingle)
{
    messageDistribute<int> dist;
    int value = 0;
    auto token = dist.addUpdateCallback([&value](int x) { value = x; });
    dist.distribute(42);
    yield_for_async();
    EXPECT_EQ(value, 42);
}

TEST_F(MessageDistributeTest, MultipleCallbacksFired)
{
    messageDistribute<int> dist;
    int a = 0, b = 0;
    dist.addUpdateCallback([&a](int x) { a = x; });
    dist.addUpdateCallback([&b](int x) { b = x; });
    dist.distribute(7);
    yield_for_async();
    EXPECT_EQ(a, 7);
    EXPECT_EQ(b, 7);
}

TEST_F(MessageDistributeTest, RemoveCallbackStopsDelivery)
{
    messageDistribute<int> dist;
    int value = 0;
    auto token = dist.addUpdateCallback([&value](int x) { value += x; });
    dist.distribute(1);
    yield_for_async();
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(dist.removeUpdateCallback(token));
    dist.distribute(10);
    EXPECT_EQ(value, 1);
}

TEST_F(MessageDistributeTest, RemoveNonExistentReturnsFalse)
{
    messageDistribute<int> dist;
    auto fake = std::make_shared<std::function<void(int)>>([](int) {});
    EXPECT_FALSE(dist.removeUpdateCallback(fake));
}

TEST_F(MessageDistributeTest, DistributeWithNoSubscribers)
{
    messageDistribute<int> dist;
    dist.distribute(42);
    SUCCEED();
}

TEST_F(MessageDistributeTest, ClearRemovesAll)
{
    messageDistribute<int> dist;
    int a = 0, b = 0;
    dist.addUpdateCallback([&a](int x) { a = x; });
    dist.addUpdateCallback([&b](int x) { b = x; });
    dist.clear();
    dist.distribute(99);
    EXPECT_EQ(a, 0);
    EXPECT_EQ(b, 0);
    EXPECT_EQ(dist.callbackCount(), 0);
}

TEST_F(MessageDistributeTest, CallbackCountReflectsState)
{
    messageDistribute<int> dist;
    EXPECT_EQ(dist.callbackCount(), 0);
    auto t1 = dist.addUpdateCallback([](int) {});
    EXPECT_EQ(dist.callbackCount(), 1);
    auto t2 = dist.addUpdateCallback([](int) {});
    EXPECT_EQ(dist.callbackCount(), 2);
    dist.removeUpdateCallback(t1);
    EXPECT_EQ(dist.callbackCount(), 1);
    dist.removeUpdateCallback(t2);
    EXPECT_EQ(dist.callbackCount(), 0);
}

TEST_F(MessageDistributeTest, MultipleTypesSupported)
{
    messageDistribute<int, std::string> dist;
    int num = 0;
    std::string str;
    dist.addUpdateCallback([&num, &str](int n, std::string s) {
        num = n;
        str = std::move(s);
    });
    dist.distribute(42, "hello");
    yield_for_async();
    EXPECT_EQ(num, 42);
    EXPECT_EQ(str, "hello");
}

TEST_F(MessageDistributeTest, TokenIdentityIsUnique)
{
    messageDistribute<int> dist;
    int a = 0, b = 0;
    auto t1 = dist.addUpdateCallback([&a](int x) { a = x; });
    auto t2 = dist.addUpdateCallback([&b](int x) { b = x; });
    EXPECT_NE(t1.get(), t2.get());
    dist.removeUpdateCallback(t1);
    dist.distribute(3);
    yield_for_async();
    EXPECT_EQ(a, 0);
    EXPECT_EQ(b, 3);
}

TEST_F(MessageDistributeTest, ExceptionInOneCallbackDoesNotBreakOthers)
{
    messageDistribute<int> dist;
    int a = 0, b = 0;
    dist.addUpdateCallback([&a](int x) {
        a = x;
        throw std::runtime_error("boom");
    });
    dist.addUpdateCallback([&b](int x) { b = x; });
    dist.distribute(5);
    yield_for_async();
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 5);
}

TEST_F(MessageDistributeTest, DistributeIsNonBlocking)
{
    messageDistribute<> dist;
    std::atomic<bool> finished{false};

    dist.addUpdateCallback([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        finished.store(true);
    });

    auto t0 = std::chrono::steady_clock::now();
    dist.distribute();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();

    EXPECT_LT(elapsed, 100)
        << "distribute() should return immediately without waiting for callbacks";

    // Give the detached thread a moment to start, then verify it is
    // still running (i.e. distribute() did not block for 2 seconds).
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(finished.load())
        << "callback should still be running after distribute() returns";
}

TEST_F(MessageDistributeTest, ThreadSafetyConcurrentSubscribeAndDistribute)
{
    messageDistribute<int> dist;
    std::atomic<int> total{0};
    constexpr int numThreads = 4;
    constexpr int iterations = 1000;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                auto token = dist.addUpdateCallback([&total](int x) {
                    total += x;
                });
                dist.distribute(1);
                dist.removeUpdateCallback(token);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    EXPECT_GE(total.load(), numThreads * iterations);
    EXPECT_EQ(dist.callbackCount(), 0);
}
