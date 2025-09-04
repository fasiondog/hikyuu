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
 * @brief 普通多任务队列线程池，任务之间彼此独立不能互相等待
 * @note 任务运行之间如存在先后顺序，请使用 StealThreadPool。
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
     * 默认构造函数，创建和当前系统CPU数一致的线程数
     */
    MQThreadPool() : MQThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * 构造函数，创建指定数量的线程
     * @param n 指定的线程数
     * @param until_empty 任务队列为空时，自动停止运行
     */
    explicit MQThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_runnging_until_empty(until_empty) {
        try {
            m_thread_need_stop.resize(m_worker_num);
            for (int i = 0; i < m_worker_num; i++) {
                // 创建工作线程及其任务队列
                m_queues.push_back(
                  std::unique_ptr<ThreadSafeQueue<task_type>>(new ThreadSafeQueue<task_type>));
            }
            // 初始完毕所有线程资源后再启动线程
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.push_back(std::thread(&MQThreadPool::worker_thread, this, i));
            }
        } catch (...) {
            m_done = true;
            throw;
        }
    }

    /**
     * 析构函数，等待并阻塞至线程池内所有任务完成
     */
    ~MQThreadPool() {
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
        size_t total = 0;
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
    auto submit(FunctionType f) {
        if (m_done) {
            throw std::logic_error("You can't submit a task to the stopped MQThreadPool!");
        }

        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());

        // 向空队列或任务数最小的队列中加入任务
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

    /** 返回线程池结束状态 */
    bool done() const {
        return m_done;
    }

    /**
     * 等待各线程完成当前执行的任务后立即结束退出
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
     * 等待并阻塞至线程池内所有任务完成
     * @note 至此线程池能工作线程结束不可再使用
     */
    void join() {
        if (m_done) {
            return;
        }

        // 指示各工作线程在未获取到工作任务时，停止运行
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

        {  // 等待线程结束
            std::lock_guard<std::mutex> lock(m_mutex_join);
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_threads[i].joinable()) {
                    m_threads[i].join();
                }
            }
        }

        m_done = true;
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;      // 线程池全局需终止指示
    size_t m_worker_num;          // 工作线程数量
    bool m_runnging_until_empty;  // 运行直到队列空时停止

    std::vector<std::unique_ptr<ThreadSafeQueue<task_type>>> m_queues;  // 线程任务队列
    std::vector<InterruptFlag> m_thread_need_stop;                      // 线程终止标志
    std::vector<std::thread> m_threads;                                 // 工作线程
    std::mutex m_mutex_join;                                            // 用于保护 joinable

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