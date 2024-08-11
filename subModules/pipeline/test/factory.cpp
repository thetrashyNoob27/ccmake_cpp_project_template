#include <iostream>
#include <vector>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <utility>

#define PIPELINE_DEBUG
#include "pipeline.hpp"
#include "sineWaveFactory.h"
#undef PIPELINE_DEBUG

// Test cases
int main()
{
    DEBUG_PRINTF("start test:  %s", __FILE__);
    const size_t len = 1000;
    const double frequencyStep=0.001;
    std::vector<std::pair<double, uint_fast32_t>> source(len);
    for (int i = 0; i < len; i++)
    {
        source[i] = std::pair<double, uint_fast32_t>(i*frequencyStep, len*1000);
    }
    sineWaveFactory factory;
    factory.setProcessWorkerCount(8);
    DEBUG_PRINTF("object worker create count: %d", factory.getWorkerCount());
    for (auto &v : source)
    {
        factory.addJob(v);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    size_t changeFlag;
    for (int i = 0; i < 10; i++)
    {
        size_t pendingCnt, processingCnt, outputCnt;
        factory.getQueueCount(pendingCnt, processingCnt, outputCnt);
        DEBUG_PRINTF("pending:%llu processing:%llu output:%llu\n", pendingCnt, processingCnt, outputCnt);
        if (pendingCnt | processingCnt)
        {
        }
        else
        {
            DEBUG_PRINTF("factory empty!");
            break;
        }
        auto thisFlag = pendingCnt + processingCnt;
        if (changeFlag != thisFlag)
        {
            i = 0;
            changeFlag = thisFlag;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    int getBackCnt = 0;
    bool resultWrong = false;
    while (true)
    {
        std::pair<double, std::vector<double>> result;
        bool getstatus = factory.getProduct(result);
        if (!getstatus)
        {
            break;
        }
        getBackCnt++;
        if (!factory.testcase_resultCheck(result.first, result.second))
        {
            resultWrong = true;
        }
    }
    if (resultWrong)
    {
        DEBUG_PRINTF("result wrong");
        return 1;
    }
    else if (getBackCnt != len)
    {
        DEBUG_PRINTF("result cnt not match how many put in");
        return 2;
    }
    else
    {
        DEBUG_PRINTF("check ok");
    }
    return 0;
}