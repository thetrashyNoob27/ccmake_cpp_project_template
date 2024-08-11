#include <iostream>
#include <vector>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <utility>

#define PIPELINE_DEBUG
#include <pipeline.hpp>

#include "delay_plus1.h"
int main()
{
    DEBUG_PRINTF("hire and fire test start");
    plusOne factory;
    factory.setProcessWorkerCount(0);
    DEBUG_PRINTF("object create finish.");
    int workerCnt = 100;
    {
        workerCnt = 2;
        DEBUG_PRINTF("worker count set to:%d", workerCnt);
        factory.setProcessWorkerCount(workerCnt);
        DEBUG_PRINTF("hire/fire finish.");
        for (const auto &v : gen_test_list(workerCnt))
        {
            factory.addJob(v);
        }
    }
    {
        workerCnt = 0;
        DEBUG_PRINTF("worker count set to:%d", workerCnt);
        factory.setProcessWorkerCount(workerCnt);
        DEBUG_PRINTF("hire/fire finish.");
        for (const auto &v : gen_test_list(workerCnt))
        {
            factory.addJob(v);
        }
    }
    {
        workerCnt = 0;
        DEBUG_PRINTF("worker count set to:%d", workerCnt);
        factory.setProcessWorkerCount(workerCnt);
        DEBUG_PRINTF("hire/fire finish.");
        for (const auto &v : gen_test_list(workerCnt))
        {
            factory.addJob(v);
        }
    }

    return 0;
}