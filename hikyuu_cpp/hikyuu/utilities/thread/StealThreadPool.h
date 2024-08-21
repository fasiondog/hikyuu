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
 * @brief 分布偷取式线程池
 * @note 主要用于存在递归情况，任务又创建任务加入线程池的情况，否则建议使用普通的线程池
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
     * 默认构造函数，创建和当前系统CPU数一致的线程数
     */
    StealThreadPool() : StealThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * 构造函数，创建指定数量的线程
     * @param n 指定的线程数
     * @param until_empty 任务队列为空时，自动停止运行
     */
    explicit StealThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_running_until_empty(until_empty) {
        try {
            m_interrupt_flags.resize(m_worker_num, nullptr);
            for (int i = 0; i < m_worker_num; i++) {
                // 创建工作线程及其任务队列
                m_queues.emplace_back(new WorkStealQueue);
            }
            // 初始完毕所有线程资源后再启动线程
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.emplace_back(&StealThreadPool::worker_thread, this, i);
            }
        } catch (...) {
            m_done = true;
            throw;
        }
    }

    /**
     * 析构函数，等待并阻塞至线程池内所有任务完成
     */
    ~StealThreadPool() {
        if (!m_done) {
            join();
        }
    }

    /** 获取工作线程数 */
    size_t worker_num() const {
        return m_worker_num;
    }

    /** 剩余任务数 */
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

    /** 先线程池提交任务后返回的对应 future 的类型 */
    template <typename ResultType>
    using task_handle = std::future<ResultType>;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    /** 向线程池提交任务 */
    template <typename FunctionType>
    task_handle<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        if (m_thread_need_stop.isSet() || m_done) {
            throw std::logic_error("You can't submit a task to the stopped StealThreadPool!!");
        }

        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());
        if (m_local_work_queue) {
            // 本地线程任务从前部入队列（递归成栈）
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

    /** 返回线程池结束状态 */
    bool done() const {
        return m_done;
    }

    /**
     * 等待各线程完成当前执行的任务后立即结束退出
     */
    void stop() {
        if (m_done) {
            return;
        }

        m_done = true;

        // 同时加入结束任务指示，以便在dll退出时也能够终止
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_interrupt_flags[i]) {
                m_interrupt_flags[i]->set();
            }
            m_queues[i]->push_front(FuncWrapper());
        }

        m_cv.notify_all();  // 唤醒所有工作线程
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
     * 等待并阻塞至线程池内所有任务完成
     * @note 至此线程池能工作线程结束不可再使用
     */
    void join() {
        if (m_done) {
            return;
        }

        // 指示各工作线程在未获取到工作任务时，停止运行
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

            m_done = true;
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_interrupt_flags[i]) {
                    m_interrupt_flags[i]->set();
                }
            }
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_master_work_queue.push(FuncWrapper());
        }

        // 唤醒所有工作线程
        m_cv.notify_all();

        // 等待线程结束
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_master_work_queue.clear();
        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->clear();
        }

        m_done = true;
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;       // 线程池全局需终止指示
    size_t m_worker_num;           // 工作线程数量
    bool m_running_until_empty;    // 任务队列为空时，自动停止运行
    std::condition_variable m_cv;  // 信号量，无任务时阻塞线程并等待
    std::mutex m_cv_mutex;         // 配合信号量的互斥量

    std::vector<InterruptFlag*> m_interrupt_flags;           // 工作线程状态
    ThreadSafeQueue<task_type> m_master_work_queue;          // 主线程任务队列
    std::vector<std::unique_ptr<WorkStealQueue> > m_queues;  // 任务队列（每个工作线程一个）
    std::vector<std::thread> m_threads;                      // 工作线程

    // 线程本地变量
#if CPP_STANDARD >= CPP_STANDARD_17
    inline static thread_local WorkStealQueue* m_local_work_queue = nullptr;  // 本地任务队列
    inline static thread_local int m_index = -1;                  // 在线程池中的序号
    inline static thread_local InterruptFlag m_thread_need_stop;  // 线程停止运行指示
#else
    static thread_local WorkStealQueue* m_local_work_queue;  // 本地任务队列
    static thread_local int m_index;                         // 在线程池中的序号
    static thread_local InterruptFlag m_thread_need_stop;    // 线程停止运行指示
#endif

    void worker_thread(int index) {
        m_interrupt_flags[index] = &m_thread_need_stop;
        m_index = index;
        m_local_work_queue = m_queues[m_index].get();
        while (!m_thread_need_stop.isSet() && !m_done) {
            run_pending_task();
        }
        m_local_work_queue = nullptr;
        m_interrupt_flags[m_index] = nullptr;
    }

    void run_pending_task() {
        // 从本地队列提前工作任务，如本地无任务则从主队列中提取任务
        // 如果主队列中提取的任务是空任务，则认为需结束本线程，否则从其他工作队列中偷取任务
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
            std::unique_lock<std::mutex> lk(m_cv_mutex);
            m_cv.wait(lk, [=] { return this->m_done || !this->m_master_work_queue.empty(); });
        }
    }

    bool pop_task_from_master_queue(task_type& task) {
        return m_master_work_queue.try_pop(task);
    }

    // cppcheck-suppress functionStatic // 屏蔽cppcheck转静态函数建议
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
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_STEALTHREADPOOL_H */
