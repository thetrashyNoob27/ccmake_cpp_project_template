#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include "messageDistribute.h"

int main()
{
    // Test 1: basic add and distribute
    {
        messageDistribute<int> dist;
        int value = 0;
        auto token = dist.addUpdateCallback([&value](int x) { value = x; });
        dist.distribute(42);
        assert(value == 42);
        std::cout << "PASS: basic add and distribute\n";
    }

    // Test 2: multiple callbacks
    {
        messageDistribute<int> dist;
        int a = 0, b = 0;
        auto t1 = dist.addUpdateCallback([&a](int x) { a = x; });
        auto t2 = dist.addUpdateCallback([&b](int x) { b = x; });
        dist.distribute(7);
        assert(a == 7);
        assert(b == 7);
        std::cout << "PASS: multiple callbacks\n";
    }

    // Test 3: remove callback stops delivery
    {
        messageDistribute<int> dist;
        int value = 0;
        auto token = dist.addUpdateCallback([&value](int x) { value += x; });
        dist.distribute(1);
        assert(value == 1);
        bool removed = dist.removeUpdateCallback(token);
        assert(removed);
        dist.distribute(2);
        assert(value == 1);
        std::cout << "PASS: remove callback stops delivery\n";
    }

    // Test 4: remove non-existent returns false
    {
        messageDistribute<int> dist;
        auto fake = std::make_shared<std::function<void(int)>>([](int) {});
        bool removed = dist.removeUpdateCallback(fake);
        assert(!removed);
        std::cout << "PASS: remove non-existent returns false\n";
    }

    // Test 5: clear removes all
    {
        messageDistribute<int> dist;
        int a = 0, b = 0;
        dist.addUpdateCallback([&a](int x) { a = x; });
        dist.addUpdateCallback([&b](int x) { b = x; });
        dist.clear();
        dist.distribute(99);
        assert(a == 0);
        assert(b == 0);
        assert(dist.callbackCount() == 0);
        std::cout << "PASS: clear removes all\n";
    }

    // Test 6: callback count
    {
        messageDistribute<int> dist;
        assert(dist.callbackCount() == 0);
        auto t1 = dist.addUpdateCallback([](int) {});
        assert(dist.callbackCount() == 1);
        auto t2 = dist.addUpdateCallback([](int) {});
        assert(dist.callbackCount() == 2);
        dist.removeUpdateCallback(t1);
        assert(dist.callbackCount() == 1);
        std::cout << "PASS: callback count\n";
    }

    // Test 7: multiple types
    {
        messageDistribute<int, std::string> dist;
        int num = 0;
        std::string str;
        dist.addUpdateCallback([&num, &str](int n, std::string s) {
            num = n;
            str = std::move(s);
        });
        dist.distribute(42, "hello");
        assert(num == 42);
        assert(str == "hello");
        std::cout << "PASS: multiple types\n";
    }

    // Test 8: exception in one callback does not break others
    {
        messageDistribute<int> dist;
        int a = 0, b = 0;
        dist.addUpdateCallback([&a](int x) {
            a = x;
            throw std::runtime_error("boom");
        });
        dist.addUpdateCallback([&b](int x) { b = x; });
        dist.distribute(5);
        assert(a == 5);
        assert(b == 5);
        std::cout << "PASS: exception isolation\n";
    }

    // Test 9: token identity is unique
    {
        messageDistribute<int> dist;
        int a = 0, b = 0;
        auto t1 = dist.addUpdateCallback([&a](int x) { a = x; });
        auto t2 = dist.addUpdateCallback([&b](int x) { b = x; });
        assert(t1.get() != t2.get());
        dist.removeUpdateCallback(t1);
        dist.distribute(3);
        assert(a == 0);
        assert(b == 3);
        std::cout << "PASS: token identity is unique\n";
    }

    std::cout << "All standalone tests passed!\n";
    return 0;
}
