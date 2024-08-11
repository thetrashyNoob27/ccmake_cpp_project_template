#ifndef __sineWaveFactory_H__
#define __sineWaveFactory_H__

#include <iostream>
#include <vector>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <utility>
#include <pipeline.hpp>
#include <random>
#include <math.h>

typedef std::pair<double, uint_fast32_t> __input_type;        // frequency , sample points
typedef std::pair<double, std::vector<double>> __output_type; // frequency , wave
class sineWaveFactory : public pipeline<__input_type, __output_type>
{
protected:
    __output_type process(const __input_type &meterial) override
    {
        auto frequency = meterial.first;
        auto cnt = meterial.second;
        std::vector<double> product(cnt);
        for (uint_fast32_t i = 0; i < cnt; i++)
        {
            product[i] = std::sin(frequency * 2 * M_PI * i);
        }
        return __output_type(frequency, product);
    }

public:
    bool testcase_resultCheck(double frequency, const std::vector<double> &product)
    {
        for (uint_fast32_t i = 0; i < product.size(); i++)
        {
            if (product.at(i) != std::sin(frequency * 2 * M_PI * i))
            {
                return false;
            }
        }
        return true;
    }
};

#endif