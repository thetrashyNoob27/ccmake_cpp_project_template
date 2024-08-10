#include <iostream>
#include <vector>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <utility>
#include <pipeline.hpp>



#include "delay_plus1.h"

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