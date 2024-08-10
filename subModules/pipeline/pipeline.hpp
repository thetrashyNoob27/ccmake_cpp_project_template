#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <queue>
#include <list>

#include <algorithm>

#include <memory>
#include <condition_variable>
#include <thread>
#include <mutex>

#include <iomanip>
#include <iostream>
#include <ostream>

#include "threadInfo.h"

#include "debug_printf.h"

template <typename Tinput, typename Toutput>
class pipeline
{
public:
    pipeline(unsigned int workerCnt = 1 /*std::thread::hardware_concurrency()*/)
    {
        setWorkerCount(workerCnt);
    }
    virtual ~pipeline()
    {
        setWorkerCount(0);
    }

    void setWorkerCount(unsigned int count)
    {
        std::lock_guard<std::mutex> lk(mutex_this);
        while (workerList.size() < count)
        {
            auto workerDocument = new threadInfo();
            workerDocument->cv_lock = &mutex_process;
            workerDocument->quit = false;
            workerDocument->cv_worker = &cv_jobAdd;
            workerDocument->worker = new std::thread(&pipeline::workerJob, this, workerDocument);

            workerList.push_back(workerDocument);
        }
        if (workerList.size() == count)
        {
            return;
        }
        {
            std::ostringstream workerAddr;
            for (const threadInfo *info : workerList)
            {
                workerAddr << std::uppercase << std::hex /* << std::setw(2 * sizeof(uintptr_t)) << std::setfill('0')*/ << reinterpret_cast<uintptr_t>(info->worker) << " ";
            }
            DEBUG_PRINTF("thread address now: %s", workerAddr.str().c_str());
        }
        while (workerList.size() > count)
        {
            auto worker = workerList.front();
            auto workerAddress = reinterpret_cast<uintptr_t>(worker->worker);
            DEBUG_PRINTF("(worker:0x%X) thread delete start", workerAddress);
            delete worker;
            DEBUG_PRINTF("(worker:0x%X) deleted finish", workerAddress);
            workerList.pop_front();
        }
    }

    unsigned int getWorkerCount()
    {
        std::lock_guard<std::mutex> lk(mutex_this);
        return workerList.size();
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
        std::lock(mutex_process, mutex_output);
        std::lock_guard<std::mutex> lock_process(mutex_process, std::adopt_lock);
        std::lock_guard<std::mutex> lock_output(mutex_output, std::adopt_lock);

        pendingCnt = queue_process.size();
        finishedCnt = queue_output.size();
        processingCnt = 0;
        std::for_each(workerList.begin(), workerList.end(),
                      [&](const threadInfo *workerProfile)
                      {
                          if (workerProfile->status == threadStatus::BUSY)
                          {
                              processingCnt++;
                          };
                      });
    }

protected:
    virtual Toutput process(const Tinput &meterial) = 0;

private:
    void workerJob(threadInfo *contract)
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
                cv_jobAdd.wait(lock_process, [&]()
                               {
                               volatile bool notGonnaLock=contract->quit || !queue_process.empty();
                               DEBUG_PRINTF("(worker:0x%X) not gonna lock? %d (quit? %d queue not-empty? %d)",reinterpret_cast<uintptr_t>(contract->worker), notGonnaLock,contract->quit,!queue_process.empty());
                                return notGonnaLock; });
                DEBUG_PRINTF("(worker:0x%X) post-cv-wait (mutex unlocked)", reinterpret_cast<uintptr_t>(contract->worker));
                contract->status = threadStatus::BUSY;
                if (contract->quit)
                {
                    DEBUG_PRINTF("(worker:0x%X) pre-return", reinterpret_cast<uintptr_t>(contract->worker));
                    contract->status = threadStatus::EXITED;
                    return;
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
                std::lock_guard<std::mutex> lock_output(mutex_output);
                queue_output.push(product);
                cv_processFinish.notify_one();
            }
            DEBUG_PRINTF("(worker:0x%X) loop end.", reinterpret_cast<uintptr_t>(contract->worker));
        }
        contract->status = threadStatus::EXITED;
    }

private: // variables
    std::mutex mutex_this;
    std::list<threadInfo *> workerList;
    std::queue<Tinput> queue_process;
    std::mutex mutex_process;
    std::condition_variable cv_jobAdd;
    std::queue<Toutput> queue_output;
    std::mutex mutex_output;
    std::condition_variable cv_processFinish;
    volatile void (*outputCallback)(Toutput &product) = nullptr;
};
#endif