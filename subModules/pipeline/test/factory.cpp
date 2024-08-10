#include <iostream>
#include <vector>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <utility>
#include <pipeline.hpp>

#define ASSERT_TRUE(cond)                                                                                  \
    do                                                                                                     \
    {                                                                                                      \
        if (!(cond))                                                                                       \
        {                                                                                                  \
            std::cerr << "Assertion failed: " #cond << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1;                                                                                      \
        }                                                                                                  \
    } while (0)

class plusOne : public pipeline<int, std::pair<int, int>>
{
protected:
    std::pair<int, int> process(const int &meterial) override
    {
        auto res = meterial + 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        auto package = std::pair<int, int>(meterial, res);
        std::printf("process package input:%d output:%d\n", package.first, package.second);
        return std::pair<int, int>(meterial, res);
    }
};

template class pipeline<int, std::pair<int, int>>;

// Test cases
int main()
{
    // Test 1
    std::cout << "start test: " << __FILE__ << std::endl;
    const size_t len = 100;
    std::vector<int> source(len);
    for (int i = 0; i < len; i++)
    {
        source[i] = i;
    }
    plusOne factory;
    std::cout << "object worker create count " << factory.getWorkerCount() << std::endl;
    for (auto &v : source)
    {
        factory.addJob(v);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    int count = 0;
    while (true)
    {
        {
            size_t pendingCnt, processingCnt, outputCnt;
            factory.getQueueCount(pendingCnt, processingCnt, outputCnt);
            std::printf("pending:%llu processing:%llu output:%llu\n", pendingCnt, processingCnt, outputCnt);
            if (pendingCnt | processingCnt | outputCnt)
            {
            }
            else
            {
                std::printf("factory empty!\n");
                break;
            }
        }
        std::pair<int, int> res;
        auto success = factory.getProduct(res);
        if (!success)
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                size_t pendingCnt, processingCnt, outputCnt;
                factory.getQueueCount(pendingCnt, processingCnt, outputCnt);
                if (outputCnt != 0)
                    break;
            }

            continue;
        }
        count++;
        if (res.first + 1 != res.second)
        {
            std::printf(" result error input:%d output:%d\n", res.first, res.second);
            return 1;
        }
    }
    if (count != len)
    {
        std::cout << "not enough get back!" << count << std::endl;
        return 1;
    }
    std::cout << "All tests passed!" << std::endl;
    return 0;
}