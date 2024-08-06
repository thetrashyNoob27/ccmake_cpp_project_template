#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

std::string getCurrentTimeString(const char* format="%Y-%m-%d %H:%M:%S")
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm now_tm = *std::localtime(&now_c);

    std::ostringstream timeStream;
    timeStream << std::put_time(&now_tm, format);

    return timeStream.str();
}