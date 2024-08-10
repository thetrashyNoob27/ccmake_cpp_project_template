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

int main()
{
    int workerCnt = 100;
    std::cout << "start test: " << __FILE__ << std::endl;
    plusOne factory;
    std::cout << "object worker set to  " << workerCnt;
    factory.setWorkerCount(workerCnt);
    std::cout << "   " << "hire/fire finish." << std::endl;
    workerCnt = 10;
    std::cout << "object worker set to  " << workerCnt;
    factory.setWorkerCount(workerCnt);
    std::cout << "   " << "hire/fire finish." << std::endl;
    workerCnt = 0;
    std::cout << "object worker set to  " << workerCnt;
    factory.setWorkerCount(workerCnt);
    std::cout << "   " << "hire/fire finish." << std::endl;
    return 0;
}