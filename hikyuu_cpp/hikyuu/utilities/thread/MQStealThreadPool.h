/*
 * StealMQStealThreadPool.h
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
#include "FuncWrapper.h"
#include "MQStealQueue.h"
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
 * @brief Multi queue task stealing pool without a centralized queue
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class MQStealThreadPool {
#else
class HKU_UTILS_API MQStealThreadPool {
#endif
public:
    /**
     * Default constructor, it creates the number of the threads equal to the number of the CPUs of
     * the current system
     */
    MQStealThreadPool() : MQStealThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * Constructor, it creates the given number of the threads
     * @param n the given number of the threads
     * @param until_empty it stops running automatically when the task queue is empty
     */
    explicit MQStealThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_runnging_until_empty(until_empty) {
        try {
            m_interrupt_flags.resize(m_worker_num);
            for (size_t i = 0; i < m_worker_num; i++) {
                // Create the worker threads and their task queues
                m_queues.emplace_back(new MQStealQueue<task_type>);
            }
            // The threads are started after all the thread resources have been initialized
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.emplace_back(&MQStealThreadPool::worker_thread, this, i);
                m_thread_index[m_threads.back().get_id()] = i;
            }
        } catch (...) {
            m_done = true;
            // Wake up and join the already started worker threads before the members are
            // destroyed, otherwise the workers would access the destroyed members
            size_t started = m_threads.size();
            for (size_t i = 0; i < started; i++) {
                m_interrupt_flags[i].set();
                m_queues[i]->push(FuncWrapper());
            }
            for (size_t i = 0; i < started; i++) {
                if (m_threads[i].joinable()) {
                    m_threads[i].join();
                }
            }
            throw;
        }
    }

    /**
     * Destructor, it waits and blocks until all the tasks in the thread pool are finished
     */
    ~MQStealThreadPool() {
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
    auto submit(FunctionType&& f) {
        if (m_done) {
            throw std::logic_error("You can't submit a task to the stopped MQStealThreadPool!");
        }

        // The counter is incremented before the task is pushed so that join() can observe an
        // in-flight task even while it is between pop and execution. A second m_done check after
        // the increment guards against the race where join() flips m_done right before the push.
        m_running_task_count.fetch_add(1, std::memory_order_relaxed);
        if (m_done) {
            _releaseRunningTask();
            throw std::logic_error("You can't submit a task to the stopped MQStealThreadPool!");
        }

        int index = -1;
        auto iter = m_thread_index.find(std::this_thread::get_id());
        if (iter != m_thread_index.end()) {
            index = iter->second;
        }

        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
        task_handle<result_type> res(task.get_future());

        try {
            // If it is the local thread and the thread has not been terminated, it is added to its
            // own queue
            if (index != -1 && m_interrupt_flags[index]) {
                // The local thread tasks enter the queue from the front (recursion becomes a stack)
                m_queues[index]->push_front(std::move(task));
                return res;
            }

            // Round-robin across worker queues; atomic fetch_add avoids the data race on the
            // index when multiple producer threads submit concurrently. Modulo keeps it in range
            // even after the counter wraps.
            int idx = m_current_index.fetch_add(1, std::memory_order_relaxed) %
                      static_cast<int>(m_worker_num);
            m_queues[idx]->push(std::move(task));
        } catch (...) {
            _releaseRunningTask();
            throw;
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
        // Serialize with join(): two threads must never join the same worker concurrently
        std::lock_guard<std::mutex> lock(m_join_mutex);
        // Reject new submissions before waking the workers, otherwise a task submitted during
        // stop could be silently dropped (and its future broken) or even executed
        if (m_done.exchange(true, std::memory_order_relaxed)) {
            return;
        }

        // Wake a concurrent joiner sleeping on the drain wait (stop discards the queued tasks,
        // so the counter alone would never reach zero)
        {
            std::lock_guard<std::mutex> lk(m_idle_mutex);
            m_idle_cv.notify_all();
        }

        // push_front the terminating null tasks so a blocked worker wakes up on the sentinel
        // itself: try_steal refuses tail null tasks, and a worker in wait_and_pop cannot observe
        // its interrupt flag until something is pushed
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_interrupt_flags[i]) {
                m_interrupt_flags[i].set();
            }
            m_queues[i]->push_front(FuncWrapper());
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_done = true;
        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->clear();
        }
    }

    /**
     * It waits and blocks until all the tasks in the thread pool are finished
     * @note From then on the thread pool cannot be used after the worker threads are ended
     */
    void join() {
        // It instructs every worker thread to stop running when no work task is got
        if (m_runnging_until_empty) {
            // Sleep-wait until there is no queued nor in-flight task instead of spinning (a long
            // running task would otherwise burn a core for the whole join). The counter already
            // covers queued tasks (incremented at submit time), so a zero counter means the pool
            // is truly idle. stop() discards queued tasks without decrementing the counter, so
            // m_done must also break the wait (stop notifies m_idle_cv when it sets m_done).
            std::unique_lock<std::mutex> lk(m_idle_mutex);
            m_idle_cv.wait(lk, [this] {
                return m_done.load(std::memory_order_acquire) ||
                       m_running_task_count.load(std::memory_order_acquire) == 0;
            });
        }

        // Serialize with other join()/stop() calls: the workers must be joined exactly once
        std::lock_guard<std::mutex> lock(m_join_mutex);
        if (m_done) {
            return;
        }

        m_done = true;

        // Wake a concurrent joiner sleeping on the drain wait
        {
            std::lock_guard<std::mutex> lk(m_idle_mutex);
            m_idle_cv.notify_all();
        }

        if (m_runnging_until_empty) {
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_interrupt_flags[i]) {
                    m_interrupt_flags[i].set();
                }
            }
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->push(FuncWrapper());
        }

        // Wait for the threads to be finished
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_done = true;
        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->clear();
        }
    }

    struct ExecutorWrapper {
        MQStealThreadPool* pool;
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

    // Balances the increment done in submit(). Wakes a joiner sleeping on the drain wait when
    // the last in-flight task finishes. The notify is done under m_idle_mutex so it cannot slip
    // through the window between the waiter's predicate evaluation and its blocking.
    void _releaseRunningTask() {
        if (m_running_task_count.fetch_sub(1, std::memory_order_relaxed) == 1) {
            std::lock_guard<std::mutex> lk(m_idle_mutex);
            m_idle_cv.notify_all();
        }
    }

    // Decrements the in-flight task counter on scope exit. Used to balance the increment done in
    // submit() once a popped task has finished executing (including when it throws).
    struct RunningTaskGuard {
        MQStealThreadPool& pool;
        explicit RunningTaskGuard(MQStealThreadPool& p) : pool(p) {}
        ~RunningTaskGuard() {
            pool._releaseRunningTask();
        }
    };

    std::atomic_bool m_done;      // The global termination indication of the thread pool
    size_t m_worker_num;          // Number of the worker threads
    bool m_runnging_until_empty;  // It runs until the queue is empty and then stops

    std::atomic<size_t> m_running_task_count{0};  // Submitted but not yet finished tasks (queued +
                                                  // in-flight)

    std::vector<std::unique_ptr<MQStealQueue<task_type>>> m_queues;  // Thread task queues
    std::vector<InterruptFlag> m_interrupt_flags;                    // Thread termination flags
    std::vector<std::thread> m_threads;                              // Worker threads
    std::mutex m_join_mutex;  // Serializes join()/stop() so workers are joined exactly once
    std::mutex m_idle_mutex;  // Pairs with m_idle_cv: join() sleeps here for the pool to drain
    std::condition_variable m_idle_cv;

    std::unordered_map<std::thread::id, int> m_thread_index;
    std::atomic<int> m_current_index{0};  // The queue index used when a new task is placed
                                          // currently (round-robin, atomic for multi-producer)

    void worker_thread(int index) {
        while (!m_interrupt_flags[index].isSet() && !m_done) {
            run_pending_task(index);
        }
    }

    void run_pending_task(int index) {
        task_type task;
        // Try to get a task from the local queue and execute it
        if (m_queues[index]->try_pop(task)) {
            if (task.isNullTask()) {
                m_interrupt_flags[index].set();
            } else {
                RunningTaskGuard guard(*this);
                task();
            }
        } else if (pop_task_from_other_thread_queue(task, index)) {
            RunningTaskGuard guard(*this);
            task();
        } else {
            // Block and wait for a new task in the local queue
            // Note: in the recursive case the tasks are preferentially added to the local queue and
            // depend on each other; if a task is stolen by another thread, the other thread would
            // be blocked and wait Therefore it waits for the local queue here instead of continuing
            // to steal in a loop
            m_queues[index]->wait_and_pop(task);
            if (task.isNullTask()) {
                m_interrupt_flags[index].set();
            } else {
                RunningTaskGuard guard(*this);
                task();
            }
        }
    }

    bool pop_task_from_other_thread_queue(task_type& task, int index) {
        for (size_t i = 0; i < m_worker_num; ++i) {
            size_t pos = (index + i + 1) % m_worker_num;
            if (pos != index && m_queues[pos]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }
};  // namespace hku

} /* namespace hku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif