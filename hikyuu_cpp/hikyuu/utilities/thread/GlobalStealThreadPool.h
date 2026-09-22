/*
 * GlobalStealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once

#include <future>
#include <thread>
#include <vector>
#include "ThreadSafeQueue.h"
#include "WorkStealQueue.h"
#include "InterruptFlag.h"
#include "../Log.h"
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
class HKU_UTILS_API GlobalStealThreadPool {
public:
    /**
     * Default constructor, it creates the number of the threads equal to the number of the CPUs of
     * the current system
     */
    GlobalStealThreadPool() : GlobalStealThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * Constructor, it creates the given number of the threads
     * @param n the given number of the threads
     * @param until_empty it stops running automatically when the task queue is empty
     */
    explicit GlobalStealThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_running_until_empty(until_empty), m_sleep_count(0) {
        try {
            m_interrupt_flags.resize(m_worker_num, nullptr);
            for (int i = 0; i < m_worker_num; i++) {
                // Create the worker threads and their task queues
                m_queues.emplace_back(new WorkStealQueue);
            }
            // The threads are started after all the thread resources have been initialized
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.emplace_back(&GlobalStealThreadPool::worker_thread, this, i);
            }
        } catch (...) {
            m_done.store(true, std::memory_order_release);
            throw;
        }
    }

    /**
     * Destructor, it waits and blocks until all the tasks in the thread pool are finished
     */
    ~GlobalStealThreadPool() {
        if (!m_done.load(std::memory_order_acquire)) {
            join();
        }
    }

    /** Get the number of the worker threads */
    size_t worker_num() const {
        return m_worker_num;
    }

    /** Get the number of the currently sleeping worker threads */
    int sleep_count() const {
        return m_sleep_count.load(std::memory_order_acquire);
    }

    /**
     * Intelligently wake up the sleeping threads
     * It adaptively judges whether a wake-up is needed according to the current number of the
     * remaining tasks and the number of the sleeping threads
     * @return the number of the actually woken threads
     */
    int wake_up() {
        HKU_IF_RETURN(m_done.load(std::memory_order_acquire), 0);
        int sleeping_count = m_sleep_count.load(std::memory_order_acquire);
        if (sleeping_count <= 0) {
            return 0;
        }

        // Get the current number of the remaining tasks
        size_t remaining_tasks = remain_task_count();
        if (remaining_tasks == 0) {
            // There is no remaining task, no wake-up is needed
            return 0;
        }

        // The intelligent wake-up strategy:
        // 1. If the number of the tasks is greater than or equal to the number of the sleeping
        //    threads, use notify_all to wake up all the threads (more efficient)
        // 2. If the number of the tasks is less than the number of the sleeping threads, wake up
        //    precisely the needed number of the threads
        int threads_to_wake = 0;
        if (remaining_tasks >= static_cast<size_t>(sleeping_count)) {
            // The tasks are sufficient, use notify_all to wake up all the sleeping threads (better
            // performance)
            m_cv.notify_all();
            threads_to_wake = sleeping_count;
        } else {
            // There are few tasks, wake up precisely as needed
            threads_to_wake = static_cast<int>(remaining_tasks);
            // Ensure that at least one thread is woken up to handle the task
            threads_to_wake = std::max(threads_to_wake, 1);

            // Wake up precisely the given number of the threads
            for (int i = 0; i < threads_to_wake; ++i) {
                m_cv.notify_one();
            }
        }

        return threads_to_wake;
    }

    /** Number of the remaining tasks */
    size_t remain_task_count() const {
        if (m_done.load(std::memory_order_acquire)) {
            return 0;
        }
        size_t total = m_master_work_queue.size();
        for (size_t i = 0; i < m_worker_num; i++) {
            total += m_queues[i]->size();
        }
        return total;
    }

    /** Whether the current thread is a worker thread */
    static bool is_work_thread() {
        return m_local_work_queue != nullptr;
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
        if (m_thread_need_stop.isSet() || m_done.load(std::memory_order_acquire)) {
            throw std::logic_error(
              "You can't submit a task to the stopped GlobalStealThreadPool!!");
        }

        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
        task_handle<result_type> res(task.get_future());

        std::thread::id id = std::this_thread::get_id();
        if (m_local_work_queue && id == m_thread_id) {
            // The local thread tasks enter the queue from the front (recursion becomes a stack)
            m_local_work_queue->push_front(std::move(task));
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
        return m_done.load(std::memory_order_acquire);
    }

    /**
     * It waits for every thread to finish the currently executed task and then exits immediately
     */
    void stop() {
        if (m_done.exchange(true, std::memory_order_acq_rel)) {
            return;
        }

        // Reset the sleep count
        m_sleep_count.store(0, std::memory_order_release);

        // At the same time the end task indication is added, so that it can also be terminated when
        // the dll exits
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_interrupt_flags[i]) {
                m_interrupt_flags[i]->set();
            }
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
        m_threads.clear();
    }

    /**
     * It waits and blocks until all the tasks in the thread pool are finished
     * @note From then on the thread pool cannot be used after the worker threads are ended
     */
    void join() {
        if (m_done.load(std::memory_order_acquire)) {
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
                        if (m_queues[i]->size() != 0) {
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

            m_done.store(true, std::memory_order_release);
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_interrupt_flags[i]) {
                    m_interrupt_flags[i]->set();
                }
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

        m_done.store(true, std::memory_order_release);
        m_master_work_queue.clear();
        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->clear();
        }
        m_threads.clear();
    }

    struct ExecutorWrapper {
        GlobalStealThreadPool* pool;
        template <typename Function>
        void execute(Function f) {
            pool->submit(std::move(f));
        }
    };

    /** Coroutine executor */
    ExecutorWrapper executor() {
        return ExecutorWrapper{this};
    }

public:
    bool run_available_task_once() {
        HKU_IF_RETURN(m_done.load(std::memory_order_acquire) || m_thread_need_stop.isSet(), false);
        bool task_run = false;
        task_type task;
        if (m_local_work_queue) {
            if (pop_task_from_local_queue(task)) {
                if (!task.isNullTask()) {
                    task();
                    task_run = true;
                } else {
                    m_thread_need_stop.set();
                }
            } else if (pop_task_from_other_thread_queue(task)) {
                task();
                task_run = true;
            } else if (pop_task_from_master_queue(task)) {
                if (!task.isNullTask()) {
                    task();
                    task_run = true;
                } else {
                    m_thread_need_stop.set();
                }
            }
        } else if (pop_task_from_master_queue(task)) {
            if (!task.isNullTask()) {
                task();
                task_run = true;
            }
        }
        return task_run;
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;         // The global termination indication of the thread pool
    size_t m_worker_num;             // Number of the worker threads
    bool m_running_until_empty;      // It stops running automatically when the task queue is empty
    std::condition_variable m_cv;    // Semaphore, it blocks the threads and waits when there is no
                                     // task
    std::mutex m_cv_mutex;           // The mutex working together with the semaphore
    std::atomic<int> m_sleep_count;  // Sleep count

    std::vector<InterruptFlag*> m_interrupt_flags;           // Worker thread states
    ThreadSafeQueue<task_type> m_master_work_queue;          // Task queue of the master thread
    std::vector<std::unique_ptr<WorkStealQueue> > m_queues;  // Task queues (one for every worker
                                                             // thread)
    std::vector<std::thread> m_threads;                      // Worker threads

// Thread local variables
#if HKU_OS_WINDOWS
    static WorkStealQueue* m_local_work_queue;  // Local task queue
    static int m_index;                         // The index in the thread pool
    static InterruptFlag m_thread_need_stop;    // The indication for stopping the thread
    static std::thread::id m_thread_id;

#else
#if CPP_STANDARD >= CPP_STANDARD_17 && !defined(__clang__)
    inline static thread_local WorkStealQueue* m_local_work_queue = nullptr;  // Local task queue
    inline static thread_local int m_index = -1;                  // The index in the thread pool
    inline static thread_local InterruptFlag m_thread_need_stop;  // The indication for stopping the
                                                                  // thread
    inline static thread_local std::thread::id m_thread_id;
#else
    static thread_local WorkStealQueue* m_local_work_queue;  // Local task queue
    static thread_local int m_index;                         // The index in the thread pool
    static thread_local InterruptFlag m_thread_need_stop;  // The indication for stopping the thread
    static thread_local std::thread::id m_thread_id;
#endif
#endif

    void worker_thread(int index) {
        m_thread_id = std::this_thread::get_id();
        m_interrupt_flags[index] = &m_thread_need_stop;
        m_index = index;
        m_local_work_queue = m_queues[index].get();
        while (!m_thread_need_stop.isSet() && !m_done.load(std::memory_order_acquire)) {
            run_pending_task();
        }
        m_local_work_queue = nullptr;
        m_interrupt_flags[index] = nullptr;
    }

    void run_pending_task() {
        // Take the work task from the local queue first; if there is no local task, take it from
        // the master queue If the task taken from the master queue is an empty task, this thread is
        // considered to be ended; otherwise a task is stolen from the other work queues
        task_type task;
        if (pop_task_from_local_queue(task)) {
            if (!task.isNullTask()) {
                task();
            } else {
                m_thread_need_stop.set();
            }
        } else if (pop_task_from_master_queue(task)) {
            if (!task.isNullTask()) {
                task();
            } else {
                m_thread_need_stop.set();
            }
        } else if (pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            // Increase the sleep count before entering the waiting state
            m_sleep_count.fetch_add(1, std::memory_order_acq_rel);

            // std::this_thread::yield();
            std::unique_lock<std::mutex> lk(m_cv_mutex);
            m_cv.wait(lk, [this] {
                return this->m_done.load(std::memory_order_acquire) ||
                       !this->m_master_work_queue.empty() ||
                       (m_local_work_queue && !m_local_work_queue->empty()) ||
                       has_other_remain_task();
            });

            // Decrease the sleep count after being woken up
            m_sleep_count.fetch_sub(1, std::memory_order_acq_rel);
        }
    }

    bool pop_task_from_master_queue(task_type& task) {
        return m_master_work_queue.try_pop(task);
    }

    // cppcheck-suppress functionStatic  // Suppress the cppcheck suggestion of converting it into a
    // static function
    bool pop_task_from_local_queue(task_type& task) {
        return m_local_work_queue && m_local_work_queue->try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task) {
        for (int i = 0; i < m_worker_num; ++i) {
            int index = (m_index + i + 1) % m_worker_num;
            if (index != m_index && m_queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

    bool has_other_remain_task() {
        for (int i = 0; i < m_worker_num; ++i) {
            if (i != m_index && m_queues[i] && !m_queues[i]->empty()) {
                return true;
            }
        }
        return false;
    }
};

} /* namespace hku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
