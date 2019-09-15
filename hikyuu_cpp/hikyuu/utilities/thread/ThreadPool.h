/*
 * ThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_THREAD_THREADPOOL_H
#define HIKYUU_UTILITIES_THREAD_THREADPOOL_H

#include <future>
#include <thread>
#include "WorkStealQueue.h"
#include "ThreadSafeQueue.h"

namespace hku {

class join_threads {
public:
    explicit join_threads(std::vector<std::thread>& threads): m_threads(threads) {}

    ~join_threads() {
        for (unsigned long i = 0; i < m_threads.size(); i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }
    }

private:
    std::vector<std::thread>& m_threads;
};

class ThreadPool {
public:
    ThreadPool(): joiner(threads),done(false) {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try {
            for(unsigned i = 0; i < thread_count; ++i) {
                queues.push_back(std::unique_ptr<WorkStealQueue>(new WorkStealQueue));
                threads.push_back(std::thread(&ThreadPool::worker_thread, this, i));
            }
        } catch(...) {
            done=true;
            throw;
        }
    }
    
    ~ThreadPool() {
        done=true;
    }

    template<typename ResultType>
    using task_handle = std::future<ResultType>;

    template<typename FunctionType>
    task_handle<std::result_of<FunctionType()>::type> submit(FunctionType f) {
        typedef std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());
        if(local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }

    void run_pending_task() {
        task_type task;
        if (pop_task_from_local_queue(task) 
          || pop_task_from_pool_queue(task) 
          || pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool done;
    ThreadSafeQueue<task_type> pool_work_queue;
    std::vector<std::unique_ptr<WorkStealQueue> > queues;
    std::vector<std::thread> threads;
    join_threads joiner;

    static thread_local WorkStealQueue* local_work_queue;
    static thread_local unsigned my_index;
   
    void worker_thread(unsigned my_index_) {
        my_index = my_index_;
        local_work_queue = queues[my_index].get();
        while (!done) {
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(task_type& task) {
        return local_work_queue && local_work_queue->try_pop(task);
    }

    bool pop_task_from_pool_queue(task_type& task) {
        return pool_work_queue.try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task) {
        for (unsigned i = 0; i < queues.size(); ++i) {
            unsigned const index=(my_index+i+1)%queues.size();
            if (queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_THREADPOOL_H */
