#include "periodicWork.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    // Test 1: Basic start/stop behavior inferred from counter
    {
        periodicWork scheduler([]() {}, 100);
        std::atomic<int> counter{0};

        periodicWork worker([&counter]() { counter++; }, 50);
        worker.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        assert(counter.load() >= 1);
        worker.stop();

        int countAfterStop = counter.load();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        assert(counter.load() == countAfterStop);

        std::cout << "[PASS] testBasicStartStop" << std::endl;
    }

    // Test 2: Periodic execution
    {
        std::atomic<int> counter{0};
        periodicWork worker([&counter]() { counter++; }, 50);

        worker.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(220));
        worker.stop();

        assert(counter >= 2 && counter <= 6);

        std::cout << "[PASS] testPeriodicExecution (counter=" << counter.load() << ")" << std::endl;
    }

    // Test 3: setPeriod changes rate
    {
        std::atomic<int> counter{0};
        periodicWork worker([&counter]() { counter++; }, 200);

        worker.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        int countSlow = counter.load();
        assert(countSlow >= 1);

        worker.setPeriod(50);
        std::this_thread::sleep_for(std::chrono::milliseconds(220));
        worker.stop();

        int countFast = counter.load();
        assert(countFast > countSlow + 1);

        std::cout << "[PASS] testSetPeriod (counter=" << counter.load() << ")" << std::endl;
    }

    // Test 4: setCallback swaps function
    {
        std::atomic<int> a{0};
        std::atomic<int> b{0};

        periodicWork worker([&a]() { a++; }, 50);

        worker.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        assert(a.load() > 0);

        worker.setCallback([&b]() { b++; });
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        worker.stop();

        assert(b.load() > 0);
        std::cout << "[PASS] testSetCallback (a=" << a.load() << ", b=" << b.load() << ")" << std::endl;
    }

    // Test 5: Destructor auto-stops
    {
        std::atomic<int> counter{0};
        {
            periodicWork worker([&counter]() { counter++; }, 50);
            worker.start();
            std::this_thread::sleep_for(std::chrono::milliseconds(120));
            assert(counter.load() >= 1);
        }
        std::cout << "[PASS] testDestructorAutoStop" << std::endl;
    }

    // Test 6: Multiple instances work independently
    {
        std::atomic<int> c1{0};
        std::atomic<int> c2{0};

        periodicWork fast([&c1]() { c1++; }, 50);
        periodicWork slow([&c2]() { c2++; }, 150);

        fast.start();
        slow.start();

        std::this_thread::sleep_for(std::chrono::milliseconds(320));

        fast.stop();
        slow.stop();

        assert(c1.load() > c2.load());
        std::cout << "[PASS] testMultipleInstances (fast=" << c1.load() << ", slow=" << c2.load() << ")" << std::endl;
    }

    std::cout << "All periodic_work tests passed." << std::endl;
    return 0;
}
