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
    std::cout << "start test: " << __FILE__ << std::endl;
    plusOne factory;
    std::printf("");
    int workerCnt = 100;
    {
        workerCnt = 10;
        std::cout << "object worker set to  " << workerCnt;
        factory.setWorkerCount(workerCnt);
        std::cout << "   " << "hire/fire finish." << std::endl;
        for (const auto &v : gen_test_list(workerCnt))
        {
            factory.addJob(v);
        }
    }
    {
        workerCnt = 0;
        std::cout << "object worker set to  " << workerCnt;
        factory.setWorkerCount(workerCnt);
        std::cout << "   " << "hire/fire finish." << std::endl;
        for (const auto &v : gen_test_list(workerCnt))
        {
            factory.addJob(v);
        }
    }
    {
        workerCnt = 0;
        std::cout << "object worker set to  " << workerCnt;
        factory.setWorkerCount(workerCnt);
        std::cout << "   " << "hire/fire finish." << std::endl;
        for (const auto &v : gen_test_list(workerCnt))
        {
            factory.addJob(v);
        }
    }

    return 0;
}