/*
 * StealMQThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once

#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include "InterruptFlag.h"
#include "FuncWrapper.h"
#include "ThreadSafeQueue.h"
#include "../cppdef.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * @brief An ordinary multi task queue thread pool; the tasks are independent of each other and
 * cannot wait for each other
 * @note If the tasks have a sequential order, please use StealThreadPool.
 * @details
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class MQThreadPool {
#else
class HKU_UTILS_API MQThreadPool {
#endif
public:
    /**
     * Default constructor, it creates the number of the threads equal to the number of the CPUs of
     * the current system
     */
    MQThreadPool() : MQThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * Constructor, it creates the given number of the threads
     * @param n the given number of the threads
     * @param until_empty it stops running automatically when the task queue is empty
     */
    explicit MQThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_runnging_until_empty(until_empty) {
        try {
            m_thread_need_stop.resize(m_worker_num);
            for (int i = 0; i < m_worker_num; i++) {
                // Create the worker threads and their task queues
                m_queues.push_back(
                  std::unique_ptr<ThreadSafeQueue<task_type>>(new ThreadSafeQueue<task_type>));
            }
            // The threads are started after all the thread resources have been initialized
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.push_back(std::thread(&MQThreadPool::worker_thread, this, i));
            }
        } catch (...) {
            m_done = true;
            throw;
        }
    }

    /**
     * Destructor, it waits and blocks until all the tasks in the thread pool are finished
     */
    ~MQThreadPool() {
        if (!m_done) {
            join();
        }
        m_threads.clear();
    }

    /** Get the number of the worker threads */
    size_t worker_num() const {
        return m_worker_num;
    }

    /** Number of the remaining tasks */
    size_t remain_task_count() const {
        size_t total = 0;
        for (size_t i = 0; i < m_worker_num; i++) {
            total += m_queues[i]->size();
        }
        return total;
    }

    /** The type of the corresponding future returned after submitting a task to the thread pool */
    template <typename ResultType>
    using task_handle = std::future<ResultType>;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    /** Submit a task to the thread pool */
    template <typename FunctionType>
    auto submit(FunctionType &&f) {
        if (m_done) {
            throw std::logic_error("You can't submit a task to the stopped MQThreadPool!");
        }

        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
        task_handle<result_type> res(task.get_future());

        // Add the task to the empty queue or the queue with the smallest number of the tasks
        size_t min_count = std::numeric_limits<size_t>::max();
        int index = 0;
        for (int i = 0; i < m_worker_num; ++i) {
            if (!m_thread_need_stop[i].isSet()) {
                size_t cur_count = m_queues[i]->size();
                if (cur_count == 0) {
                    index = i;
                    break;
                }

                if (cur_count < min_count) {
                    min_count = cur_count;
                    index = i;
                }
            }
        }

        m_queues[index]->push(std::move(task));
        return res;
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    /** Return the end state of the thread pool */
    bool done() const {
        return m_done;
    }

    /**
     * It waits for every thread to finish the currently executed task and then exits immediately
     */
    void stop() {
        if (m_done.exchange(true, std::memory_order_relaxed)) {
            return;
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_thread_need_stop[i].set();
            m_queues[i]->push(FuncWrapper());
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex_join);
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_threads[i].joinable()) {
                    m_threads[i].join();
                }
            }
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->clear();
        }
    }

    /**
     * It waits and blocks until all the tasks in the thread pool are finished
     * @note From then on the thread pool cannot be used after the worker threads are ended
     */
    void join() {
        if (m_done) {
            return;
        }

        // It instructs every worker thread to stop running when no work task is got
        if (!m_runnging_until_empty) {
            m_done = true;
            for (size_t i = 0; i < m_worker_num; i++) {
                m_thread_need_stop[i].set();
            }
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->push(FuncWrapper());
            m_queues[i]->notify_all();
        }

        {  // Wait for the threads to be finished
            std::lock_guard<std::mutex> lock(m_mutex_join);
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_threads[i].joinable()) {
                    m_threads[i].join();
                }
            }
        }

        m_done = true;
    }

    struct ExecutorWrapper {
        MQThreadPool *pool;
        template <typename Function>
        void execute(Function f) {
            pool->submit(std::move(f));
        }
    };

    /** Coroutine executor */
    ExecutorWrapper executor() {
        return ExecutorWrapper{this};
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;      // The global termination indication of the thread pool
    size_t m_worker_num;          // Number of the worker threads
    bool m_runnging_until_empty;  // It runs until the queue is empty and then stops

    std::vector<std::unique_ptr<ThreadSafeQueue<task_type>>> m_queues;  // Thread task queues
    std::vector<InterruptFlag> m_thread_need_stop;                      // Thread termination flags
    std::vector<std::thread> m_threads;                                 // Worker threads
    std::mutex m_mutex_join;                                            // Used to protect joinable

    void worker_thread(int index) {
        auto *local_queue = m_queues[index].get();
        auto *local_stop_flag = &m_thread_need_stop[index];
        while (!local_stop_flag->isSet() || !m_done) {
            task_type task;
            local_queue->wait_and_pop(task);
            if (task.isNullTask()) {
                local_stop_flag->set();
                break;
            }
            task();
        }
    }
};

} /* namespace hku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif