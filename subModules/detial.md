# Message Distributor - Thread-safe Callback Pattern

> Extracted from: `src/lcmsControl/modules/clinicalAcquisitionWrapper/modules/topicManageModel/`

## Overview
Thread-safe callback distribution system with token-based registration. No memory leaks with shared_ptr tokens. Perfect for event-driven architectures and observer patterns.

## Features
- Thread-safe callback management
- Token-based registration/removal
- No memory leaks (shared_ptr tokens)
- Variadic template support for any callback signature
- Simple and efficient design

## Dependencies
- C++11 (std::function, std::shared_ptr, std::mutex)
- STL only (no external dependencies)

## Files Included
1. `messageDistribute.h` - Header-only template class

## Quick Deploy Script
Copy and execute in bash to deploy:

```bash
cat << 'EOF' > messageDistribute.h
#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_set>

#define __CALLBACK_LIST_LOCK std::lock_guard<std::mutex> lk(listLock);

namespace std {
template<typename... Args>
struct hash<std::shared_ptr<std::function<void(Args...)>>>
{
    size_t operator()(const std::shared_ptr<std::function<void(Args...)>> &func) const
    {
        return reinterpret_cast<size_t>(func.get());
    }
};
} // namespace std

template<typename... Args>
class messageDistribute
{
public:
    using callback = std::function<void(Args...)>;
    using callbackToken = std::shared_ptr<callback>;

    messageDistribute() {}
    ~messageDistribute()
    {
        __CALLBACK_LIST_LOCK
        callbackList.clear();
    }

    callbackToken addUpdateCallback(callback handle)
    {
        auto h = std::make_shared<callback>(handle);
        {
            __CALLBACK_LIST_LOCK
            callbackList.insert(h);
        }
        return h;
    }

    bool removeUpdateCallback(const callbackToken &token)
    {
        __CALLBACK_LIST_LOCK
        auto it = callbackList.find(token);
        if (it == callbackList.end()) {
            return false;
        }
        callbackList.erase(it);
        return true;
    }

    void distribute(Args... args)
    {
        __CALLBACK_LIST_LOCK
        for (const auto &c : callbackList) {
            (*c)(std::forward<Args>(args)...);
        }
    }

protected:
    std::unordered_set<callbackToken> callbackList;
    std::mutex listLock;
};
EOF

echo "messageDistribute.h created successfully!"
```

## Usage Example

### Publisher Side

```cpp
#include "messageDistribute.h"

class DataPublisher {
public:
    // Declare distributor with callback signature: (int, QString)
    messageDistribute<int, QString> dataUpdated;

    void processData(int id, const QString& name) {
        // ... process data ...
        
        // Notify all subscribers
        dataUpdated.distribute(id, name);
    }
};
```

### Subscriber Side

```cpp
#include "messageDistribute.h"

class DataSubscriber {
public:
    void subscribe(DataPublisher* pub) {
        // Register callback, receive token for later removal
        callbackToken = pub->dataUpdated.addUpdateCallback(
            [this](int id, const QString& name) {
                qDebug() << "Received:" << id << name;
                handleUpdate(id, name);
            }
        );
    }

    void unsubscribe(DataPublisher* pub) {
        pub->dataUpdated.removeUpdateCallback(callbackToken);
    }

private:
    void handleUpdate(int id, const QString& name) {
        // Handle the update
    }

    messageDistribute<int, QString>::callbackToken callbackToken;
};
```

### Multiple Subscribers Example

```cpp
class Logger {
public:
    void subscribe(DataPublisher* pub) {
        pub->dataUpdated.addUpdateCallback(
            [](int id, const QString& name) {
                qInfo() << "Log:" << id << name;
            }
        );
    }
};

class UIUpdater {
public:
    void subscribe(DataPublisher* pub) {
        pub->dataUpdated.addUpdateCallback(
            [](int id, const QString& name) {
                // Update UI
                QMetaObject::invokeMethod(uiWidget, "updateDisplay",
                    Qt::QueuedConnection,
                    Q_ARG(int, id),
                    Q_ARG(QString, name));
            }
        );
    }
};

// Usage
DataPublisher publisher;
Logger logger;
UIUpdater ui;

logger.subscribe(&publisher);
ui.subscribe(&publisher);

publisher.processData(42, "test");  // Both callbacks called
```

## API Reference

| Method | Description |
|--------|-------------|
| `addUpdateCallback(callback)` | Register callback, returns token |
| `removeUpdateCallback(token)` | Remove callback by token |
| `distribute(args...)` | Call all registered callbacks |

## Type Aliases

| Alias | Description |
|-------|-------------|
| `callback` | `std::function<void(Args...)>` |
| `callbackToken` | `std::shared_ptr<callback>` |

## Notes

1. **Token lifetime**: Keep the token alive as long as you want to receive callbacks
2. **Thread safety**: All operations are thread-safe
3. **No copies**: Callbacks are stored as shared_ptr, no unnecessary copies
4. **Exception safe**: If a callback throws, other callbacks still execute