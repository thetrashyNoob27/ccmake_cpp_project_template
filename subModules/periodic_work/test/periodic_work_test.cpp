#include "periodicWork.h"
#include <cassert>
#include <chrono>
#include <iostream>

static void testBasicStartStop()
{
    periodicWork worker([]() {}, 100);
    assert(!worker.isRunning());

    worker.start();
    assert(worker.isRunning());

    worker.stop();
    assert(!worker.isRunning());

    std::cout << "[PASS] testBasicStartStop" << std::endl;
}

static void testPeriodicExecution()
{
    int counter = 0;
    periodicWork worker([&counter]() {
        ++counter;
    }, 50);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(220));
    worker.stop();

    assert(counter >= 2);
    assert(counter <= 6);
    std::cout << "[PASS] testPeriodicExecution (counter=" << counter << ")" << std::endl;
}

static void testSetPeriod()
{
    int counter = 0;
    periodicWork worker([&counter]() {
        ++counter;
    }, 200);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    int countAt200ms = counter;
    assert(countAt200ms >= 1);

    worker.setPeriod(50);
    std::this_thread::sleep_for(std::chrono::milliseconds(220));
    worker.stop();

    assert(counter > countAt200ms + 1);
    std::cout << "[PASS] testSetPeriod (counter=" << counter << ")" << std::endl;
}

static void testSetCallback()
{
    int a = 0;
    int b = 0;

    periodicWork worker([&a]() {
        ++a;
    }, 50);

    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    assert(a > 0);

    worker.setCallback([&b]() {
        ++b;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    worker.stop();

    assert(b > 0);
    std::cout << "[PASS] testSetCallback (a=" << a << ", b=" << b << ")" << std::endl;
}

static void testDestructorAutoStop()
{
    int counter = 0;
    {
        periodicWork worker([&counter]() {
            ++counter;
        }, 50);

        worker.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        assert(worker.isRunning());
    }
    std::cout << "[PASS] testDestructorAutoStop" << std::endl;
}

static void testMultipleInstances()
{
    int c1 = 0;
    int c2 = 0;

    periodicWork fast([&c1]() { ++c1; }, 50);
    periodicWork slow([&c2]() { ++c2; }, 150);

    fast.start();
    slow.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(320));

    fast.stop();
    slow.stop();

    assert(c1 > c2);
    std::cout << "[PASS] testMultipleInstances (fast=" << c1 << ", slow=" << c2 << ")" << std::endl;
}

int main()
{
    testBasicStartStop();
    testPeriodicExecution();
    testSetPeriod();
    testSetCallback();
    testDestructorAutoStop();
    testMultipleInstances();

    std::cout << "All periodic_work tests passed." << std::endl;
    return 0;
}
