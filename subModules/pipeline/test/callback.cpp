#include <iostream>
#include <vector>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <utility>

#define PIPELINE_DEBUG
#include "debug_printf.h"
#undef PIPELINE_DEBUG

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
    factory.setWorkerCount(10);
    std::cout << "object worker create count " << factory.getWorkerCount() << std::endl;

    DEBUG_PRINTF("begin callback set");
    bool fail = false;
    int resultCnt = 0;
    factory.setCallback([&](std::pair<int, int> &result)
                        {
                            resultCnt++;
                                    size_t pendingCnt, processingCnt, outputCnt;
                                    factory.getQueueCount(pendingCnt, processingCnt, outputCnt);
        DEBUG_PRINTF("(result callback) pending:%llu processing:%llu output:%llu\n", pendingCnt, processingCnt, outputCnt);
            if (result.first + 1 != result.second)
            {
                DEBUG_PRINTF("result error input:%d output:%d\n", result.first, result.second);
                fail = true;
            } });
    DEBUG_PRINTF("finish callback set");
    for (auto &v : source)
    {
        factory.addJob(v);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    for (int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        size_t pendingCnt, processingCnt, outputCnt;
        factory.getQueueCount(pendingCnt, processingCnt, outputCnt);
        DEBUG_PRINTF("pending:%llu processing:%llu output:%llu\n", pendingCnt, processingCnt, outputCnt);
        if (pendingCnt | processingCnt | outputCnt)
        {
        }
        else
        {
            DEBUG_PRINTF("factory empty!");
            break;
        }
    }

    if (fail)
    {
        DEBUG_PRINTF("result assert fail");
        return 1;
    }
    else if (resultCnt != len)
    {
        DEBUG_PRINTF("result cnt not match put in assert fail(should be: %d but get back: %d)", len, resultCnt);
    }
    DEBUG_PRINTF("test success");
    return 0;
}