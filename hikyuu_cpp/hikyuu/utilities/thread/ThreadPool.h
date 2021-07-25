/*
 * StealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_THREAD_THREADPOOL_H
#define HIKYUU_UTILITIES_THREAD_THREADPOOL_H

#include <cstdio>
#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include "FuncWrapper.h"
#include "ThreadSafeQueue.h"

namespace hku {

/**
 * @brief 普通集中式任务队列线程池，任务之间彼此独立不能互相等待
 * @note 任务运行之间如存在先后顺序，请使用 StealThreadPool。
 * @details
 * @ingroup ThreadPool
 */
class ThreadPool {
public:
    /**
     * 默认构造函数，创建和当前系统CPU数一致的线程数
     */
    ThreadPool() : ThreadPool(std::thread::hardware_concurrency(), true) {}

    /**
     * 构造函数，创建指定数量的线程
     * @param n 指定的线程数
     */
    explicit ThreadPool(size_t n, bool util_empty = true)
    : m_done(false), m_worker_num(n), m_runnging_util_empty(util_empty) {
        try {
            for (int i = 0; i < m_worker_num; i++) {
                m_threads_status.push_back(nullptr);
                m_threads.push_back(std::thread(&ThreadPool::worker_thread, this, i));
            }
        } catch (...) {
            m_done = true;
            throw;
        }
    }

    /**
     * 析构函数，等待并阻塞至线程池内所有任务完成
     */
    ~ThreadPool() {
        if (!m_done) {
            join();
        }
    }

    /** 获取工作线程数 */
    size_t worker_num() const {
        return m_worker_num;
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
        if (m_thread_need_stop || m_done) {
            throw std::logic_error("Can't submit a task to the stopped ThreadPool!");
        }

        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());
        m_master_work_queue.push(std::move(task));
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
            if (m_threads_status[i]) {
                m_threads_status[i]->store(true);
            }
            m_master_work_queue.push(FuncWrapper());
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
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
        if (m_runnging_util_empty) {
            while (m_master_work_queue.size() != 0) {
                std::this_thread::yield();
            }
            m_done = true;
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_threads_status[i]) {
                    m_threads_status[i]->store(true);
                }
            }
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_master_work_queue.push(std::move(FuncWrapper()));
        }

        // 等待线程结束
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_done = true;
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;     // 线程池全局需终止指示
    size_t m_worker_num;         // 工作线程数量
    bool m_runnging_util_empty;  // 运行直到队列空时停止

    std::vector<std::atomic_bool *> m_threads_status;  // 工作线程状态
    ThreadSafeQueue<task_type> m_master_work_queue;    // 主线程任务队列
    std::vector<std::thread> m_threads;                // 工作线程

    // 线程本地变量
    inline static thread_local std::atomic_bool m_thread_need_stop = false;  // 线程停止运行指示
    inline static thread_local int m_index = -1;                             // 工作线程序号

    void worker_thread(int index) {
        m_index = index;
        m_threads_status[index] = &m_thread_need_stop;
        m_thread_need_stop = false;
        while (!m_thread_need_stop && !m_done) {
            run_pending_task();
            // std::this_thread::yield();
        }
        // fmt::print("thread ({}) finished!\n", std::this_thread::get_id());
    }

    void run_pending_task() {
        task_type task;
        m_master_work_queue.wait_and_pop(task);
        if (task.isNullTask()) {
            m_thread_need_stop = true;
        } else {
            task();
        }
    }

};  // namespace hku

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_THREADPOOL_H */
