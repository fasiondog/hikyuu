/*
 * StealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_THREAD_STEALTHREADPOOL_H
#define HIKYUU_UTILITIES_THREAD_STEALTHREADPOOL_H

#include <future>
#include <thread>
#include <vector>
#include "ThreadSafeQueue.h"
#include "WorkStealQueue.h"
#include "InterruptFlag.h"
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
 * @brief Distributed stealing thread pool
 * @note It is mainly used in the recursive case where a task creates further tasks and adds them to
 *       the thread pool; otherwise an ordinary thread pool is recommended
 * @details
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class StealThreadPool {
#else
class HKU_UTILS_API StealThreadPool {
#endif
public:
    /**
     * Default constructor, it creates the number of the threads equal to the number of the CPUs of
     * the current system
     */
    StealThreadPool() : StealThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * Constructor, it creates the given number of the threads
     * @param n the given number of the threads
     * @param until_empty it stops running automatically when the task queue is empty
     */
    explicit StealThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_running_until_empty(until_empty) {
        try {
            m_interrupt_flags.resize(m_worker_num);
            for (int i = 0; i < m_worker_num; i++) {
                // Create the worker threads and their task queues
                m_queues.emplace_back(new WorkStealQueue);
            }
            // The threads are started after all the thread resources have been initialized
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.emplace_back(&StealThreadPool::worker_thread, this, i);
                m_thread_index[m_threads.back().get_id()] = i;
            }
        } catch (...) {
            m_done = true;
            throw;
        }
    }

    /**
     * Destructor, it waits and blocks until all the tasks in the thread pool are finished
     */
    ~StealThreadPool() {
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
        if (m_done) {
            return 0;
        }
        size_t total = m_master_work_queue.size();
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
    auto submit(FunctionType&& f) {
        if (m_done) {
            throw std::logic_error("You can't submit a task to the stopped StealThreadPool!!");
        }

        int index = -1;
        auto iter = m_thread_index.find(std::this_thread::get_id());
        if (iter != m_thread_index.end()) {
            index = iter->second;
        }

        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
        task_handle<result_type> res(task.get_future());
        if (index != -1 && !m_interrupt_flags[index]) {
            // The local thread tasks enter the queue from the front (recursion becomes a stack)
            m_queues[index]->push_front(std::move(task));
        } else {
            m_master_work_queue.push(std::move(task));
            m_cv.notify_one();
        }
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
        if (m_done) {
            return;
        }

        m_done = true;

        // At the same time the end task indication is added, so that it can also be terminated when
        // the dll exits
        for (size_t i = 0; i < m_worker_num; i++) {
            m_interrupt_flags[i].set();
            m_queues[i]->push_front(FuncWrapper());
        }

        m_cv.notify_all();  // Wake up all the worker threads
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_master_work_queue.clear();
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
        if (m_running_until_empty) {
            while (true) {
                if (m_master_work_queue.size() != 0) {
                    std::this_thread::yield();
                } else {
                    bool can_quit = true;
                    for (size_t i = 0; i < m_worker_num; i++) {
                        if (!m_queues[i]->empty()) {
                            can_quit = false;
                            break;
                        }
                    }
                    if (can_quit) {
                        break;
                    } else {
                        std::this_thread::yield();
                    }
                }
            }

            m_done = true;
            for (size_t i = 0; i < m_worker_num; i++) {
                m_interrupt_flags[i].set();
            }
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_master_work_queue.push(FuncWrapper());
        }

        // Wake up all the worker threads
        m_cv.notify_all();

        // Wait for the threads to be finished
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_done = true;
        m_master_work_queue.clear();
        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->clear();
        }
    }

    struct ExecutorWrapper {
        StealThreadPool* pool;
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
    std::condition_variable
      m_cv;                 // Semaphore, it blocks the threads and waits when there is no task
    std::mutex m_cv_mutex;  // The mutex working together with the semaphore

    std::vector<InterruptFlag> m_interrupt_flags;           // Worker thread states
    ThreadSafeQueue<task_type> m_master_work_queue;         // Task queue of the master thread
    std::vector<std::unique_ptr<WorkStealQueue>> m_queues;  // Task queues (one for every worker
                                                            // thread)
    std::vector<std::thread> m_threads;                     // Worker threads
    std::unordered_map<std::thread::id, int> m_thread_index;

    void worker_thread(int index) {
        while (!m_done && !m_interrupt_flags[index]) {
            run_pending_task(index);
        }
    }

    void run_pending_task(int index) {
        // Take the work task from the local queue first; if there is no local task, take it from
        // the master queue If the task taken from the master queue is an empty task, this thread is
        // considered to be ended; otherwise a task is stolen from the other work queues
        task_type task;
        if (pop_task_from_local_queue(task, index)) {
            if (!task.isNullTask()) {
                task();
            } else {
                m_interrupt_flags[index].set();
            }
        } else if (pop_task_from_master_queue(task)) {
            if (!task.isNullTask()) {
                task();
            } else {
                m_interrupt_flags[index].set();
            }
        } else if (pop_task_from_other_thread_queue(task, index)) {
            task();
        } else {
            std::unique_lock<std::mutex> lk(m_cv_mutex);
            m_cv.wait(lk, [this] { return this->m_done || !this->m_master_work_queue.empty(); });
        }
    }

    bool pop_task_from_master_queue(task_type& task) {
        return m_master_work_queue.try_pop(task);
    }

    // cppcheck-suppress functionStatic  // Suppress the cppcheck suggestion of converting it into a
    // static function
    bool pop_task_from_local_queue(task_type& task, int index) {
        return m_queues[index]->try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task, int index) {
        for (int i = 0; i < m_worker_num; ++i) {
            int pos = (index + i + 1) % m_worker_num;
            if (pos != index && !m_interrupt_flags[pos] && m_queues[pos]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_STEALTHREADPOOL_H */
