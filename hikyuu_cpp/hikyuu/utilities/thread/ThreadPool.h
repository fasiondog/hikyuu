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
#include <vector>
#include "WorkStealQueue.h"

namespace hku {

/**
 * @brief 线程池
 * @details
 * @ingroup Utilities
 */
class ThreadPool {
public:
    /**
     * 默认构造函数，创建和当前系统CPU数一致的线程数
     */
    ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * 构造函数，创建指定数量的线程
     * @param n 指定的线程数
     */
    explicit ThreadPool(size_t n)
    : m_done(false), m_init_finished(false), m_worker_num(n), m_current_index(0) {
        try {
            for (size_t i = 0; i < m_worker_num; i++) {
                // 创建工作线程的任务队列
                m_queues.push_back(std::unique_ptr<WorkStealQueue>(new WorkStealQueue));

                // 创建工作线程，并获取线程局部变量 m_local_finish_until_empty 的地址
                std::promise<bool*> promise;
                m_threads.push_back(
                  std::thread(&ThreadPool::worker_thread, this, i, std::ref(promise)));
                m_finish_until_empty.push_back(promise.get_future().get());
            }
        } catch (...) {
            m_done = true;
            throw;
        }
        m_init_finished = true;
    }

    /**
     * 析构函数，等待各线程完成内部任务并结束退出
     */
    ~ThreadPool() {
        m_done = true;
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }
    }

    /** 获取工作线程数 */
    size_t worker_num() const {
        return m_worker_num;
    }

    /** 先线程池提交任务后返回的对应 future 的类型 */
    template <typename ResultType>
    using task_handle = std::future<ResultType>;

    /** 向线程池提交任务 */
    template <typename FunctionType>
    task_handle<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());
        if (m_local_work_queue) {
            // 本地线程任务从前部入队列（递归成栈）
            m_local_work_queue->push_front(std::move(task));
        } else {
            //主线程任务从后部入队列（FIFO）
            m_queues[m_current_index++]->push_back(std::move(task));
            if (m_current_index >= m_worker_num) {
                m_current_index = 0;
            }
        }
        return res;
    }

    /**
     * 等待并阻塞至线程池内所有任务完成
     * @note 至此线程池能工作线程结束不可再使用
     */
    void join() {
        // 指示各工作线程在未获取到工作任务时，停止运行
        for (size_t i = 0; i < m_worker_num; i++) {
            (*m_finish_until_empty[i]) = true;
        }

        // 等待线程结束
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;  // 线程池全局需终止指示
    bool m_init_finished;     // 线程池是否初始化完毕
    size_t m_worker_num;      // 工作线程数量
    size_t m_current_index;   // 指示从主线程提交任务时应提交至哪一个工作线程

    std::vector<bool*> m_finish_until_empty;  // 指示无任务时终止线程（每个工作线程一个）
    std::vector<std::unique_ptr<WorkStealQueue> > m_queues;  // 任务队列（每个工作线程一个）
    std::vector<std::thread> m_threads;                      // 工作线程

    // 线程本地变量
    inline static thread_local WorkStealQueue* m_local_work_queue = nullptr;  // 本地任务队列
    inline static thread_local size_t m_index = 0;                       //在线程池中的序号
    inline static thread_local bool m_local_finish_until_empty = false;  // 未获取到任务时终止标识
    inline static thread_local bool m_thread_need_stop = false;  // 线程停止运行指示

    void worker_thread(size_t index, std::promise<bool*>& promise) {
        m_thread_need_stop = false;
        m_index = index;
        m_local_work_queue = m_queues[m_index].get();
        m_local_finish_until_empty = false;
        promise.set_value(&m_local_finish_until_empty);
        while (!m_thread_need_stop && !m_done) {
            run_pending_task();
        }
    }

    void run_pending_task() {
        // 从本地队列提前工作任务，如本地无任务则从其他工作队列偷取任务
        // 如果都没有获取到工作任务，则检测 m_local_finish_until_empty 标识判断是否需要停止运行
        task_type task;
        if (pop_task_from_local_queue(task) || pop_task_from_other_thread_queue(task)) {
            task();
        } else if (m_local_finish_until_empty) {
            m_thread_need_stop = true;
        } else {
            std::this_thread::yield();
        }
    }

    bool pop_task_from_local_queue(task_type& task) {
        return m_local_work_queue && m_local_work_queue->try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task) {
        // 线程池尚未初始化化完成时，其他任务队列可能尚未创建
        // 此时不能从其他队列偷取任务
        if (!m_init_finished) {
            return false;
        }
        for (size_t i = 0; i < m_worker_num; ++i) {
            size_t index = (m_index + i + 1) % m_worker_num;
            if (m_queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_THREADPOOL_H */
