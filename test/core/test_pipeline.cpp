#include <gtest/gtest.h>
#include <pipeline.hpp>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

// Simple concrete pipeline for testing: doubles an int
class DoublerPipeline : public pipeline<int, int>
{
protected:
    int process(const int &material) override
    {
        return material * 2;
    }
};

// Pair-producing pipeline for callback testing
class PairPipeline : public pipeline<int, std::pair<int, int>>
{
protected:
    std::pair<int, int> process(const int &material) override
    {
        return {material, material + 1};
    }
};

class PipelineBasicTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(PipelineBasicTest, DefaultWorkerCountIsOne)
{
    DoublerPipeline p;
    EXPECT_EQ(p.getWorkerCount(), 1u);
}

TEST_F(PipelineBasicTest, CanScaleWorkers)
{
    DoublerPipeline p;
    p.setProcessWorkerCount(4);
    EXPECT_EQ(p.getWorkerCount(), 4u);

    p.setProcessWorkerCount(2);
    EXPECT_EQ(p.getWorkerCount(), 2u);

    p.setProcessWorkerCount(0);
    EXPECT_EQ(p.getWorkerCount(), 0u);
}

TEST_F(PipelineBasicTest, SingleJobProcessing)
{
    DoublerPipeline p;
    p.addJob(21);

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int result;
    EXPECT_TRUE(p.getProduct(result));
    EXPECT_EQ(result, 42);

    EXPECT_FALSE(p.getProduct(result));
}

TEST_F(PipelineBasicTest, MultipleJobsAllReturnCorrectResults)
{
    DoublerPipeline p;
    p.setProcessWorkerCount(4);

    const int jobCount = 100;
    for (int i = 0; i < jobCount; ++i)
    {
        p.addJob(i);
    }

    // Wait for all jobs to finish
    for (int i = 0; i < 50; ++i)
    {
        size_t pending, processing, finished;
        p.getQueueCount(pending, processing, finished);
        if (pending == 0 && processing == 0)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::vector<int> results;
    int val;
    while (p.getProduct(val))
    {
        results.push_back(val);
    }

    EXPECT_EQ(results.size(), static_cast<size_t>(jobCount));
    for (int i = 0; i < jobCount; ++i)
    {
        bool found = false;
        for (int r : results)
        {
            if (r == i * 2)
            {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Missing expected result for input " << i;
    }
}

TEST_F(PipelineBasicTest, QueueCountsAreAccurate)
{
    DoublerPipeline p;
    p.setProcessWorkerCount(2);

    size_t pending, processing, finished;

    // Initially all zero
    p.getQueueCount(pending, processing, finished);
    EXPECT_EQ(pending, 0u);
    EXPECT_EQ(processing, 0u);
    EXPECT_EQ(finished, 0u);

    // Add jobs without waiting
    for (int i = 0; i < 10; ++i)
    {
        p.addJob(i);
    }

    // At this point, some may be pending, some processing, some finished
    p.getQueueCount(pending, processing, finished);
    EXPECT_EQ(pending + processing + finished, 10u);
}

TEST_F(PipelineBasicTest, GetProductReturnsFalseWhenEmpty)
{
    DoublerPipeline p;
    int val = -1;
    EXPECT_FALSE(p.getProduct(val));
    EXPECT_EQ(val, -1); // val should remain unchanged
}

TEST_F(PipelineBasicTest, ZeroWorkersStillQueuesJobs)
{
    DoublerPipeline p;
    p.setProcessWorkerCount(0);
    EXPECT_EQ(p.getWorkerCount(), 0u);

    p.addJob(5);
    p.addJob(6);

    size_t pending, processing, finished;
    p.getQueueCount(pending, processing, finished);
    EXPECT_EQ(pending, 2u);
    EXPECT_EQ(processing, 0u);
    EXPECT_EQ(finished, 0u);

    // Now hire workers and let them process
    p.setProcessWorkerCount(2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    int val;
    EXPECT_TRUE(p.getProduct(val));
    EXPECT_TRUE(val == 10 || val == 12);
}

TEST_F(PipelineBasicTest, CallbackIsInvokedForEachJob)
{
    PairPipeline p;
    p.setProcessWorkerCount(4);

    std::atomic<int> callbackCount{0};
    std::atomic<bool> callbackFail{false};

    auto cb = [&](std::pair<int, int> &result)
    {
        callbackCount++;
        if (result.first + 1 != result.second)
        {
            callbackFail = true;
        }
    };

    p.setCallback(cb);
    p.setCallbackWorkerCount(2);

    const int jobCount = 50;
    for (int i = 0; i < jobCount; ++i)
    {
        p.addJob(i);
    }

    // Wait for callbacks to fire
    for (int i = 0; i < 100; ++i)
    {
        size_t pending, processing, finished;
        p.getQueueCount(pending, processing, finished);
        if (pending == 0 && processing == 0 && finished == 0 && callbackCount == jobCount)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(callbackCount.load(), jobCount);
    EXPECT_FALSE(callbackFail.load());
}

TEST_F(PipelineBasicTest, ManyWorkersProcessAllJobs)
{
    DoublerPipeline p;
    p.setProcessWorkerCount(16);

    const int jobCount = 500;
    for (int i = 0; i < jobCount; ++i)
    {
        p.addJob(i);
    }

    // Poll until done
    for (int i = 0; i < 200; ++i)
    {
        size_t pending, processing, finished;
        p.getQueueCount(pending, processing, finished);
        if (pending == 0 && processing == 0)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    int val;
    int count = 0;
    while (p.getProduct(val))
    {
        count++;
    }

    EXPECT_EQ(count, jobCount);
}

TEST_F(PipelineBasicTest, DestructorDoesNotDeadlock)
{
    {
        DoublerPipeline p;
        p.setProcessWorkerCount(8);
        for (int i = 0; i < 100; ++i)
        {
            p.addJob(i);
        }
        // Destructor fires while jobs may still be processing
        // If this test hangs, there's a deadlock in ~pipeline() or ~threadInfo()
    }
    SUCCEED();
}
