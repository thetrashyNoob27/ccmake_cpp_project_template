#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

template<typename... Args>
class messageDistribute
{
public:
    using callback = std::function<void(Args...)>;
    using callbackToken = std::shared_ptr<callback>;

    messageDistribute() = default;
    ~messageDistribute() = default;

    callbackToken addUpdateCallback(callback handle)
    {
        auto h = std::make_shared<callback>(std::move(handle));
        std::lock_guard<std::mutex> lk(listLock);
        callbackList.push_back(h);
        return h;
    }

    bool removeUpdateCallback(const callbackToken &token)
    {
        if (!token) return false;
        std::lock_guard<std::mutex> lk(listLock);
        for (auto it = callbackList.begin(); it != callbackList.end(); ++it) {
            if (it->get() == token.get()) {
                callbackList.erase(it);
                return true;
            }
        }
        return false;
    }

    void distribute(Args... args)
    {
        std::vector<callbackToken> localList;
        {
            std::lock_guard<std::mutex> lk(listLock);
            localList = callbackList;
        }
        for (const auto &c : localList) {
            if (c && *c) {
                try {
                    (*c)(args...);
                } catch (...) {
                    // Continue to next callback
                }
            }
        }
    }

    size_t callbackCount() const
    {
        std::lock_guard<std::mutex> lk(listLock);
        return callbackList.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lk(listLock);
        callbackList.clear();
    }

protected:
    std::vector<callbackToken> callbackList;
    mutable std::mutex listLock;
};
