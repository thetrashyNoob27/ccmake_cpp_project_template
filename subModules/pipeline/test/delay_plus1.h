#ifndef __DELAY_PLUS1_H_
#define __DELAY_PLUS1_H_

#include <iostream>
#include <vector>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <utility>
#include <pipeline.hpp>
#include <random>

class plusOne : public pipeline<int, std::pair<int, int>>
{
protected:
    std::pair<int, int> process(const int &meterial) override
    {
        auto res = meterial + 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        auto package = std::pair<int, int>(meterial, res);
        return std::pair<int, int>(meterial, res);
    }
};

template class pipeline<int, std::pair<int, int>>;

std::vector<int> gen_test_list(int size, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max())
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);

    std::vector<int> result(size);
    std::for_each(result.begin(), result.end(), [&](int &obj)
                  { obj = dist(rng); });
    return result;
}

#endif