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
#include <chrono>
#include "WorkStealQueue.h"
#include "ThreadSafeQueue.h"

namespace hku {

class ThreadPool {
public:
    ThreadPool(): ThreadPool(std::thread::hardware_concurrency()) {}
    ThreadPool(size_t n): m_done(false), m_init_finished(false), m_worker_num(n), m_current_index(0), m_count(0) {
        try {
            for (size_t i = 0; i < m_worker_num; i++) {
                std::promise<bool *> promise;
                m_queues.push_back(std::unique_ptr<WorkStealQueue>(new WorkStealQueue));
                m_threads.push_back(std::thread(&ThreadPool::worker_thread, this, i, std::ref(promise)));
                m_finish_until_empty.push_back(promise.get_future().get());
            }
        } catch (...) {
            m_done = true;
            throw;
        }
        m_init_finished = true;
    }

    ~ThreadPool() {
        m_done = true;
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }
    }

    size_t worker_num() const {
        return m_worker_num;
    }

    template<typename ResultType>
    using task_handle = std::future<ResultType>;

    template<typename FunctionType>
    task_handle<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());
        if (m_local_work_queue) {
            m_local_work_queue->push(std::move(task));
        } else {
            m_queues[m_current_index++]->push(std::move(task));
            if (m_current_index >= m_worker_num) {
                m_current_index = 0;
            }
            //m_pool_work_queue.push(std::move(task));
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
            //auto now = std::chrono::steady_clock::now();
            if (m_local_finish_until_empty) {
                //auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(now - m_pre_time).count();
                //if (duration >= 10) {
                    m_thread_need_stop = true;
                //} else {
                //    std::this_thread::yield();    
                //}
            } else {
                //m_pre_time = now;
                std::this_thread::yield();
            }
        }
        /*if (pop_task_from_local_queue(task) || pop_task_from_other_thread_queue(task)) {
            task();
        } else if (pop_task_from_pool_queue(task)) {
            if (task.is_stop_task()) {
                m_thread_need_stop = true;
            } else {
                task();
            }
        } else {
            std::this_thread::yield();
        }*/
    }

    void join() {
        for (size_t i = 0; i < m_worker_num; i++) {
            //m_pool_work_queue.push(FuncWrapper());
            (*m_finish_until_empty[i]) = true;
        }
        
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;
    bool m_init_finished;
    size_t m_worker_num;
    ThreadSafeQueue<task_type> m_pool_work_queue;
    std::vector<bool *> m_finish_until_empty;
    std::vector<std::unique_ptr<WorkStealQueue> > m_queues;
    std::vector<std::thread> m_threads;
    size_t m_current_index;
    size_t m_count;

    inline static thread_local WorkStealQueue* m_local_work_queue = nullptr;
    inline static thread_local size_t m_index = 0;
    inline static thread_local bool m_local_finish_until_empty = false;
    inline static thread_local bool m_thread_need_stop = false;
    inline static thread_local auto m_pre_time = std::chrono::steady_clock::now();
   
    void worker_thread(size_t index, std::promise<bool *>& promise) {
        m_thread_need_stop = false;
        m_index = index;
        m_local_work_queue = m_queues[m_index].get();
        m_local_finish_until_empty = false;
        promise.set_value(&m_local_finish_until_empty);
        while (!m_thread_need_stop && !m_done) {
            run_pending_task();
        }
        std::cout << fmt::format("finished!\n");
    }

    bool pop_task_from_local_queue(task_type& task) {
        return m_local_work_queue && m_local_work_queue->try_pop(task);
    }

    bool pop_task_from_pool_queue(task_type& task) {
        return m_pool_work_queue.try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task) {
        if (!m_init_finished) {
            return false;
        }
        for (size_t i = 0; i < m_worker_num; ++i) {
            size_t index = (m_index+i+1) % m_worker_num;
            //std::cout << fmt::format("steal({})\n", m_count++); 
            if (m_queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_THREADPOOL_H */
