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
 * @brief 无集中队列多队列偷取任务池
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class MQStealThreadPool {
#else
class HKU_UTILS_API MQStealThreadPool {
#endif
public:
    /**
     * 默认构造函数，创建和当前系统CPU数一致的线程数
     */
    MQStealThreadPool() : MQStealThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * 构造函数，创建指定数量的线程
     * @param n 指定的线程数
     * @param until_empty 任务队列为空时，自动停止运行
     */
    explicit MQStealThreadPool(size_t n, bool until_empty = true)
    : m_done(false), m_worker_num(n), m_runnging_until_empty(until_empty) {
        try {
            m_interrupt_flags.resize(m_worker_num);
            for (size_t i = 0; i < m_worker_num; i++) {
                // 创建工作线程及其任务队列
                m_queues.emplace_back(new MQStealQueue<task_type>);
            }
            // 初始完毕所有线程资源后再启动线程
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.emplace_back(&MQStealThreadPool::worker_thread, this, i);
                m_thread_index[m_threads.back().get_id()] = i;
            }
        } catch (...) {
            m_done = true;
            throw;
        }
    }

    /**
     * 析构函数，等待并阻塞至线程池内所有任务完成
     */
    ~MQStealThreadPool() {
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
            throw std::logic_error("You can't submit a task to the stopped MQStealThreadPool!");
        }

        int index = -1;
        auto iter = m_thread_index.find(std::this_thread::get_id());
        if (iter != m_thread_index.end()) {
            index = iter->second;
        }

        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());

        // 如果是本地线程且线程仍未终止，则加入自身队列
        if (index != -1 && m_interrupt_flags[index]) {
            // 本地线程任务从前部入队列（递归成栈）
            m_queues[index]->push_front(std::move(task));
            return res;
        }

        m_queues[m_current_index]->push(std::move(task));
        m_current_index++;
        if (m_current_index >= m_worker_num) {
            m_current_index = 0;
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

        // 同时加入结束任务指示，以便在dll退出时也能够终止
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_interrupt_flags[i]) {
                m_interrupt_flags[i].set();
            }
            m_queues[i]->push(FuncWrapper());
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
     * 等待并阻塞至线程池内所有任务完成
     * @note 至此线程池能工作线程结束不可再使用
     */
    void join() {
        if (m_done) {
            return;
        }

        // 指示各工作线程在未获取到工作任务时，停止运行
        if (m_runnging_until_empty) {
            while (true) {
                bool can_quit = true;
                for (size_t i = 0; i < m_worker_num; i++) {
                    if (!m_queues[i]->empty()) {
                        can_quit = false;
                        break;
                    }
                }

                if (can_quit) {
                    break;
                }

                std::this_thread::yield();
            }

            m_done = true;
            for (size_t i = 0; i < m_worker_num; i++) {
                if (m_interrupt_flags[i]) {
                    m_interrupt_flags[i].set();
                }
            }
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->push(FuncWrapper());
        }

        // 等待线程结束
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

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;      // 线程池全局需终止指示
    size_t m_worker_num;          // 工作线程数量
    bool m_runnging_until_empty;  // 运行直到队列空时停止

    std::vector<std::unique_ptr<MQStealQueue<task_type>>> m_queues;  // 线程任务队列
    std::vector<InterruptFlag> m_interrupt_flags;                    // 线程终止标志
    std::vector<std::thread> m_threads;                              // 工作线程

    std::unordered_map<std::thread::id, int> m_thread_index;
    int m_current_index = 0;  // 当前放置新任务时的队列索引

    void worker_thread(int index) {
        while (!m_interrupt_flags[index].isSet() && !m_done) {
            run_pending_task(index);
        }
    }

    void run_pending_task(int index) {
        task_type task;
        // 尝试从本地队列获取任务并执行
        if (m_queues[index]->try_pop(task)) {
            if (task.isNullTask()) {
                m_interrupt_flags[index].set();
            } else {
                task();
            }
        } else if (pop_task_from_other_thread_queue(task, index)) {
            task();
        } else {
            // 阻塞并等待本地队列中有新的任务
            // 注：如果是递归情况，任务被优先加入本地队列且彼此依赖，如果任务被其他线程偷取到的话，将导致其他线程被阻塞并等待
            // 所以，此处等待本地队列而不是继续循环偷取
            m_queues[index]->wait_and_pop(task);
            if (task.isNullTask()) {
                m_interrupt_flags[index].set();
            } else {
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