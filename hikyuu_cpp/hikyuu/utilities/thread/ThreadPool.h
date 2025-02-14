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
#include <vector>
#include "FuncWrapper.h"
#include "ThreadSafeQueue.h"
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
 * @brief 普通集中式任务队列线程池，任务之间彼此独立不能互相等待
 * @note 任务运行之间如存在先后顺序，请使用 StealThreadPool。
 * @details
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class ThreadPool {
#else
class HKU_UTILS_API ThreadPool {
#endif
public:
    /**
     * 默认构造函数，创建和当前系统CPU数一致的线程数
     */
    ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * 构造函数，创建指定数量的线程
     * @param n 指定的线程数
     * @param until_empty join时，等待任务队列为空后停止运行
     */
    explicit ThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_running_until_empty(until_empty) {
        try {
            m_interrupt_flags.resize(m_worker_num, nullptr);
            // 初始完毕所有线程资源后再启动线程
            for (int i = 0; i < m_worker_num; i++) {
                // 创建工作线程及其任务队列
                m_threads.emplace_back(&ThreadPool::worker_thread, this, i);
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
    auto submit(FunctionType f) {
        if (m_thread_need_stop.isSet() || m_done) {
            throw std::logic_error("You can't submit a task to the stopped task group!");
        }
        typedef typename std::invoke_result<FunctionType>::type result_type;
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

    /** 剩余任务数 */
    size_t remain_task_count() const {
        return m_master_work_queue.size();
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
            m_master_work_queue.push(FuncWrapper());
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_master_work_queue.clear();
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
            while (m_master_work_queue.size() > 0) {
                std::this_thread::yield();
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

        // 等待线程结束
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }

        m_master_work_queue.clear();
        m_done = true;
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;     // 线程池全局需终止指示
    size_t m_worker_num;         // 工作线程数量
    bool m_running_until_empty;  // 任务队列为空时，自动停止运行

    ThreadSafeQueue<task_type> m_master_work_queue;  // 主线程任务队列
    std::vector<std::thread> m_threads;              // 工作线程
    std::vector<InterruptFlag*> m_interrupt_flags;   // 线程中断标志

    // 线程本地变量
#if CPP_STANDARD >= CPP_STANDARD_17
    inline static thread_local InterruptFlag m_thread_need_stop;  // 线程停止运行指示
    inline static thread_local int m_index = -1;                  // 在线程池中的序号
#else
    static thread_local InterruptFlag m_thread_need_stop;  // 线程停止运行指示
    static thread_local int m_index;                       // 在线程池中的序号
#endif

    void worker_thread(int index) {
        m_index = index;
        m_interrupt_flags[index] = &m_thread_need_stop;
        while (!m_thread_need_stop.isSet() && !m_done) {
            run_pending_task();
            // std::this_thread::yield();
        }
        m_interrupt_flags[index] = nullptr;
    }

    void run_pending_task() {
        task_type task;
        m_master_work_queue.wait_and_pop(task);
        if (task.isNullTask()) {
            m_thread_need_stop.set();
        } else {
            task();
        }
    }
};  // namespace hku

} /* namespace hku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif /* HIKYUU_UTILITIES_THREAD_THREADPOOL_H */
