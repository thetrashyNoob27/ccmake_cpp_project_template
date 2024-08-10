#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <queue>
#include <list>

#include <algorithm>

#include <memory>
#include <condition_variable>
#include <thread>
#include <mutex>

#include "threadInfo.h"

template <typename Tinput, typename Toutput>
class pipeline
{
public:
    pipeline()
    {
        setWorkerCount(std::thread::hardware_concurrency());
    }
    virtual ~pipeline()
    {
        setWorkerCount(0);
    }

    void setWorkerCount(unsigned int count)
    {
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
        while (workerList.size() > count)
        {
            auto worker_it = workerList.begin();
            delete (*worker_it);
            workerList.erase(worker_it);
        }
    }

    unsigned int getWorkerCount()
    {
        return workerList.size();
    }
    void addJob(const Tinput &meterial)
    {

        std::unique_lock<std::mutex> lock_process(mutex_process);
        queue_process.push(meterial);

        cv_jobAdd.notify_one();
    }

    bool getProduct(Toutput &dst)
    {
        std::unique_lock<std::mutex> lock_output(mutex_output);
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
                std::unique_lock<std::mutex> lock_process(mutex_process);
                std::printf("thread %llu wait\n", contract->worker);
                cv_jobAdd.wait(lock_process, [&]()
                               { return contract->quit.load() || !queue_process.empty(); });
                std::printf("thread %llu unlocked\n", contract->worker);
                contract->status = threadStatus::BUSY;
                if (contract->quit)
                {
                    std::printf("thread %llu  quit\n", contract->worker);
                    contract->status = threadStatus::EXITED;
                    lock_process.unlock();
                    return;
                }
                else if (queue_process.empty())
                {
                    lock_process.unlock();
                    std::printf("thread %llu  queue empty\n", contract->worker);
                    continue;
                }
                jobInfo = std::move(queue_process.front());
                queue_process.pop();

                std::printf("thread %llu  queue take\n", contract->worker);
                lock_process.unlock();
            }
            Toutput product = std::move(process(jobInfo));

            {
                std::lock_guard<std::mutex> lock_output(mutex_output);
                queue_output.push(product);
                cv_processFinish.notify_one();
            }
        }
    }

private: // variables
    std::list<threadInfo *> workerList;

    std::queue<Tinput> queue_process;
    std::mutex mutex_process;
    std::condition_variable cv_jobAdd;
    std::queue<Toutput> queue_output;
    std::mutex mutex_output;
    std::condition_variable cv_processFinish;
    void (*outputCallback)(Toutput &product) = nullptr;
};
#endif