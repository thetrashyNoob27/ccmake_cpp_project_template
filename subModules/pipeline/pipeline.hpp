#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <queue>
#include <list>

#include <algorithm>

#include <memory>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include <iomanip>
#include <iostream>
#include <ostream>

#include <functional>

#include "threadInfo.h"

#include "debug_printf.h"

template <typename Tinput, typename Toutput>
class pipeline
{
public:
    pipeline(unsigned int workerCnt = 1 /*std::thread::hardware_concurrency()*/)
    {
        // create callback worker
        setCallbackWorkerCount(1);
        // create process worker
        setProcessWorkerCount(workerCnt);
    }
    virtual ~pipeline()
    {
        {
            std::lock_guard<std::mutex> lk(mutex_process);
            decltype(queue_process) empty;
            std::swap(queue_process, empty);
            DEBUG_PRINTF("input queue cleared(remain:%d)", queue_process.size());
        }
        setProcessWorkerCount(0);
        DEBUG_PRINTF("main worker all fired");
        {
            std::lock_guard<std::mutex> lk(mutex_output);
            decltype(queue_output) empty;
            std::swap(queue_output, empty);
            DEBUG_PRINTF("output queue cleared (remain:%d)", queue_output.size());
        }

        setCallbackWorkerCount(0);
        DEBUG_PRINTF("callback worker all fired");
    }

    void setProcessWorkerCount(unsigned int count)
    {
        // void hireFireWorker(unsigned int count, std::list<threadInfo *> &workerList, std::condition_variable &cv, std::mutex &m, std::function<std::thread *(threadInfo*)> creater)
        auto _creator = [&](threadInfo *doc)
        {
            return new std::thread(&pipeline::processWorkerJob, this, doc);
        };

        hireFireWorker(count, processWorkerList, cv_jobAdd, mutex_process, _creator);
    }

    void setCallbackWorkerCount(unsigned int count)
    {
        auto _creator = [&](threadInfo *doc)
        {
            return new std::thread(&pipeline::callbackJob, this, doc);
        };
        hireFireWorker(count, callbackWorkerList, cv_processFinish, mutex_output, _creator);
    }

    unsigned int getWorkerCount()
    {
        std::lock_guard<std::mutex> lk(mutex_this);
        return processWorkerList.size();
    }
    void addJob(const Tinput &meterial)
    {
        std::lock_guard<std::mutex> lock_process(mutex_process);
        queue_process.push(meterial);
        cv_jobAdd.notify_one();
    }

    bool getProduct(Toutput &dst)
    {
        std::lock_guard<std::mutex> lock_output(mutex_output);
        if (queue_output.empty())
            return false;
        dst = std::move(queue_output.front());
        queue_output.pop();
        return true;
    }
    void getQueueCount(size_t &pendingCnt, size_t &processingCnt, size_t &finishedCnt)
    {
        std::scoped_lock lck{mutex_process, mutex_output};

        pendingCnt = queue_process.size();
        finishedCnt = queue_output.size();
        processingCnt = 0;
        std::for_each(processWorkerList.begin(), processWorkerList.end(),
                      [&](const threadInfo *workerProfile)
                      {
                          if (workerProfile->status == threadStatus::BUSY)
                          {
                              processingCnt++;
                          };
                      });
    }

    void setCallback(std::function<void(Toutput &product)> method)
    {
        std::unique_lock<std::shared_mutex> lk(mutex_outputCallback);
        outputCallback = method;
    }

protected:
    virtual Toutput process(const Tinput &meterial) = 0;

private:
    void hireFireWorker(unsigned int count, std::list<threadInfo *> &workerList, std::condition_variable &cv, std::mutex &m, std::function<std::thread *(threadInfo *)> creater)
    {

        while (workerList.size() < count)
        {
            auto workerDocument = new threadInfo();
            workerDocument->cv_lock = &m;
            workerDocument->quit = false;
            workerDocument->cv_worker = &cv;
            workerDocument->worker = creater(workerDocument);
            workerList.push_back(workerDocument);
        }

        {
            std::ostringstream workerAddr;
            for (const threadInfo *info : callbackWorkerList)
            {
                workerAddr << std::uppercase << std::hex /* << std::setw(2 * sizeof(uintptr_t)) << std::setfill('0')*/ << reinterpret_cast<uintptr_t>(info->worker) << " ";
            }
            DEBUG_PRINTF("avaiable thread address now: %s", workerAddr.str().c_str());
        }
        if (callbackWorkerList.size() == count)
        {
            return;
        }
        // fire callback worker
        while (workerList.size() > count)
        {
            auto worker = workerList.front();
            auto workerAddress = reinterpret_cast<uintptr_t>(worker->worker);
            DEBUG_PRINTF("(worker:0x%X) thread delete start", workerAddress);
            delete worker;
            DEBUG_PRINTF("(worker:0x%X) deleted finish", workerAddress);
            workerList.pop_front();
        }
        {
            std::ostringstream workerAddr;
            for (const threadInfo *info : workerList)
            {
                workerAddr << std::uppercase << std::hex /* << std::setw(2 * sizeof(uintptr_t)) << std::setfill('0')*/ << reinterpret_cast<uintptr_t>(info->worker) << " ";
            }
            DEBUG_PRINTF("(after fire worker)thread address now: %s", workerAddr.str().c_str());
        }
    }
    void processWorkerJob(threadInfo *contract)
    {
        Tinput jobInfo;
        contract->status = threadStatus::IDLE;
        while (!contract->quit)
        {
            contract->status = threadStatus::IDLE;
            {
                DEBUG_PRINTF("(worker:0x%X) worker unique locked", reinterpret_cast<uintptr_t>(contract->worker));
                std::unique_lock<std::mutex> lock_process(mutex_process);
                DEBUG_PRINTF("(worker:0x%X) enter wait(lock will be released)", reinterpret_cast<uintptr_t>(contract->worker));
                auto lockCkech = [&]()
                {
                    volatile bool notGonnaLock = contract->quit || !queue_process.empty();
                    DEBUG_PRINTF("(worker:0x%X) not gonna lock? %d (quit? %d queue not-empty? %d)", reinterpret_cast<uintptr_t>(contract->worker), notGonnaLock, contract->quit, !queue_process.empty());
                    return notGonnaLock;
                };
                cv_jobAdd.wait(lock_process, lockCkech);
                DEBUG_PRINTF("(worker:0x%X) post-cv-wait (mutex unlocked)", reinterpret_cast<uintptr_t>(contract->worker));
                contract->status = threadStatus::BUSY;
                if (contract->quit)
                {
                    DEBUG_PRINTF("(worker:0x%X) pre-return", reinterpret_cast<uintptr_t>(contract->worker));
                    break;
                }
                else if (queue_process.empty())
                {
                    DEBUG_PRINTF("(worker:0x%X) queue empty. going sleep", reinterpret_cast<uintptr_t>(contract->worker));
                    continue;
                }
                else
                {
                    jobInfo = std::move(queue_process.front());
                    queue_process.pop();
                    DEBUG_PRINTF("(worker:0x%X) queue take", reinterpret_cast<uintptr_t>(contract->worker));
                }
            }
            DEBUG_PRINTF("(worker:0x%X) call process function", reinterpret_cast<uintptr_t>(contract->worker));
            Toutput product = std::move(process(jobInfo));
            DEBUG_PRINTF("(worker:0x%X) process function finished", reinterpret_cast<uintptr_t>(contract->worker));
            {
                DEBUG_PRINTF("(worker:0x%X) start lock mutex_output", reinterpret_cast<uintptr_t>(contract->worker));
                std::lock_guard<std::mutex> lock_output(mutex_output);
                DEBUG_PRINTF("(worker:0x%X) mutex_output locked", reinterpret_cast<uintptr_t>(contract->worker));
                queue_output.push(product);
            }
            if (nullptr != outputCallback)
            {
                cv_processFinish.notify_one();
            }
            DEBUG_PRINTF("(worker:0x%X) loop end.", reinterpret_cast<uintptr_t>(contract->worker));
        }
        contract->status = threadStatus::EXITED;
    }

    void callbackJob(threadInfo *contract)
    {
        contract->status = threadStatus::IDLE;
        while (!contract->quit)
        {
            Toutput jobResult;
            contract->status = threadStatus::IDLE;
            {
                std::unique_lock<std::mutex> lock_process(mutex_output);
                auto lockCheck = [&]()
                {
                    if (!mutex_outputCallback.try_lock_shared())
                    {
                        return false; // gonna keep blocking
                    }
                    volatile bool notGonnaLock = contract->quit || (!queue_output.empty()) && (nullptr != outputCallback);
                    mutex_outputCallback.unlock_shared();
                    return notGonnaLock;
                };
                cv_processFinish.wait(lock_process, lockCheck);

                contract->status = threadStatus::BUSY;
                if (contract->quit)
                {
                    break;
                }
                else if (queue_output.empty())
                {
                    continue;
                }
                else
                {
                    jobResult = queue_output.front();
                    queue_output.pop();
                }
            }
            bool callbackFail = false;
            {
                std::shared_lock<std::shared_mutex> readLock(mutex_outputCallback);

                if (nullptr != outputCallback)
                {
                    outputCallback(jobResult);
                }
                else
                {
                    callbackFail = true;
                }
            }
            if (callbackFail)
            {
                std::lock_guard<std::mutex> lk(mutex_output);
                queue_output.push(jobResult);
            }
        }
        contract->status = threadStatus::EXITED;
    }

private: // variables
    std::mutex mutex_this;
    std::list<threadInfo *> processWorkerList;
    std::queue<Tinput> queue_process;
    std::mutex mutex_process;
    std::condition_variable cv_jobAdd;
    std::queue<Toutput> queue_output;
    std::mutex mutex_output;
    std::condition_variable cv_processFinish;

    std::list<threadInfo *> callbackWorkerList;
    std::function<void(Toutput &product)> outputCallback;
    std::shared_mutex mutex_outputCallback;
};
#endif