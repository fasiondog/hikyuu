/*
 * StealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once

#include <cstdio>
#include <future>
#include <thread>
#include <vector>
#include "FuncWrapper.h"
#include "ThreadSafeQueue.h"
#include "InterruptFlag.h"
#include "../cppdef.h"
#include "../Log.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace hku {

/**
 * @brief An ordinary centralized task queue thread pool; the tasks are independent of each other
 * and cannot wait for each other
 * @note If the tasks have a sequential order, please use StealThreadPool.
 * @details
 * @ingroup ThreadPool
 */
class ThreadPool {
public:
    /**
     * Default constructor, it creates the number of the threads equal to the number of the CPUs of
     * the current system
     */
    ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * Constructor, it creates the given number of the threads
     * @param n the given number of the threads
     * @param until_empty when joining, it waits for the task queue to be empty and then stops
     *                    running
     */
    explicit ThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_running_until_empty(until_empty) {
        try {
            // The threads are started after all the thread resources have been initialized
            for (int i = 0; i < m_worker_num; i++) {
                // Create the worker threads and their task queues
                m_threads.emplace_back(&ThreadPool::worker_thread, this, i);
            }
        } catch (...) {
            m_done = true;
            throw;
        }
    }

    /**
     * Destructor, it waits and blocks until all the tasks in the thread pool are finished
     */
    ~ThreadPool() {
        if (!m_done) {
            join();
        }
        m_threads.clear();
    }

    /** Get the number of the worker threads */
    size_t worker_num() const {
        return m_worker_num;
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
    auto submit(FunctionType&& f) {
        if (m_done) {
            throw std::logic_error("You can't submit a task to the stopped task group!");
        }
        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
        task_handle<result_type> res(task.get_future());
        m_master_work_queue.push(std::move(task));
        return res;
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    /** Return the end state of the thread pool */
    bool done() const {
        return m_done;
    }

    /** Number of the remaining tasks */
    size_t remain_task_count() const {
        return m_master_work_queue.size();
    }

    /**
     * It waits for every thread to finish the currently executed task and then exits immediately
     */
    void stop() {
        if (m_done.exchange(true, std::memory_order_relaxed)) {
            return;
        }

        // At the same time the end task indication is added, so that it can also be terminated when
        // the dll exits
        for (size_t i = 0; i < m_worker_num; i++) {
            m_master_work_queue.push(FuncWrapper());
        }

        m_master_work_queue.notify_all();

        {
            std::lock_guard<std::mutex> lock(m_mutex_join);
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_threads[i].joinable()) {
                    m_threads[i].join();
                }
            }
        }

        m_master_work_queue.clear();
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
        if (!m_running_until_empty) {
            m_done = true;
        }

        // It is still possible that some thread does not get it and thus is not terminated
        for (size_t i = 0; i < 2 * m_worker_num; i++) {
            m_master_work_queue.push(FuncWrapper());
        }

        m_master_work_queue.notify_all();

        {
            std::lock_guard<std::mutex> lock(m_mutex_join);
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_threads[i].joinable()) {
                    m_threads[i].join();
                }
            }
        }

        m_done = true;
        m_master_work_queue.clear();
    }

    struct ExecutorWrapper {
        ThreadPool* pool;
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
    std::atomic_bool m_done;     // The global termination indication of the thread pool
    size_t m_worker_num;         // Number of the worker threads
    bool m_running_until_empty;  // It stops running automatically when the task queue is empty

    ThreadSafeQueue<task_type> m_master_work_queue;  // Task queue of the master thread
    std::vector<std::thread> m_threads;              // Worker threads
    std::mutex m_mutex_join;                         // Used to protect joinable

    void worker_thread(int index) {
        while (!m_done) {
            task_type task;
            m_master_work_queue.wait_and_pop(task);
            if (task.isNullTask()) {
                break;
            }
            task();
        }
    }

};  // namespace hku

} /* namespace hku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
