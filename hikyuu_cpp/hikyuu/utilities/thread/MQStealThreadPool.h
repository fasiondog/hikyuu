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
 * @ingroup MQStealThreadPool
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
     * @param exit_thread_callback 工作线程结束时回调函数
     */
    explicit MQStealThreadPool(size_t n, bool util_empty = true)
    : m_done(false), m_worker_num(n), m_runnging_util_empty(util_empty) {
        try {
            m_interrupt_flags.resize(m_worker_num, nullptr);
            for (size_t i = 0; i < m_worker_num; i++) {
                // 创建工作线程及其任务队列
                m_queues.emplace_back(new MQStealQueue<task_type>);
            }
            // 初始完毕所有线程资源后再启动线程
            for (int i = 0; i < m_worker_num; i++) {
                m_threads.emplace_back(&MQStealThreadPool::worker_thread, this, i);
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
    task_handle<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        if (m_thread_need_stop.isSet() || m_done) {
            throw std::logic_error("You can't submit a task to the stopped MQStealThreadPool!");
        }

        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());

        // 如果是本地线程且线程仍未终止，则加入自身队列
        if (m_local_work_queue) {
            // 本地线程任务从前部入队列（递归成栈）
            m_local_work_queue->push_front(std::move(task));
            return res;
        }

        // 选择任务最少的任务队列，将任务加入其尾部
        size_t min_count = std::numeric_limits<size_t>::max();
        int index = 0;
        for (int i = 0; i < m_worker_num; ++i) {
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
        if (m_done) {
            return;
        }

        m_done = true;

        // 同时加入结束任务指示，以便在dll退出时也能够终止
        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_interrupt_flags[i]) {
                m_interrupt_flags[i]->set();
            }
            m_queues[i]->push(FuncWrapper());
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
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
        if (m_runnging_util_empty) {
            while (true) {
                bool can_quit = true;
                for (size_t i = 0; i < m_worker_num; i++) {
                    if (m_queues[i]->size() != 0) {
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
                    m_interrupt_flags[i]->set();
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

        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->clear();
        }

        m_done = true;
    }

private:
    typedef FuncWrapper task_type;
    std::atomic_bool m_done;     // 线程池全局需终止指示
    size_t m_worker_num;         // 工作线程数量
    bool m_runnging_util_empty;  // 运行直到队列空时停止

    std::vector<std::unique_ptr<MQStealQueue<task_type>>> m_queues;  // 线程任务队列
    std::vector<InterruptFlag*> m_interrupt_flags;                   // 线程终止标志
    std::vector<std::thread> m_threads;                              // 工作线程

    // 线程本地变量
#if CPP_STANDARD >= CPP_STANDARD_17
    inline static thread_local MQStealQueue<task_type>* m_local_work_queue =
      nullptr;                                                    // 本地任务队列
    inline static thread_local int m_index = -1;                  // 在线程池中的序号
    inline static thread_local InterruptFlag m_thread_need_stop;  // 线程停止运行指示
#else
    static thread_local MQStealQueue<task_type>* m_local_work_queue;  // 本地任务队列
    static thread_local int m_index;                                  // 在线程池中的序号
    static thread_local InterruptFlag m_thread_need_stop;             // 线程停止运行指示
#endif

    void worker_thread(int index) {
        m_index = index;
        m_interrupt_flags[index] = &m_thread_need_stop;
        m_local_work_queue = m_queues[m_index].get();
        while (!m_thread_need_stop.isSet() && !m_done) {
            run_pending_task();
        }
        m_interrupt_flags[m_index] = nullptr;
        m_local_work_queue = nullptr;
        // printf("%zu thread (%lld) finished!\n", m_index, std::this_thread::get_id());
    }

    void run_pending_task() {
        task_type task;
        // 尝试从本地队列获取任务并执行
        if (m_local_work_queue->try_pop(task)) {
            if (task.isNullTask()) {
                m_thread_need_stop.set();
            } else {
                task();
                // std::this_thread::yield();
            }
            return;
        }

        // 尝试从其他任务队列偷取任务
        if (pop_task_from_other_thread_queue(task)) {
            task();
            // std::this_thread::yield();
            return;
        }

        // 阻塞并等待本地队列中有新的任务
        // 注：如果是递归情况，任务被优先加入本地队列且彼此依赖，如果任务被其他线程偷取到的话，将导致其他线程被阻塞并等待
        // 所以，此处等待本地队列而不是继续循环偷取
        m_local_work_queue->wait_and_pop(task);
        if (task.isNullTask()) {
            m_thread_need_stop.set();
        } else {
            task();
            // std::this_thread::yield();
        }
    }

    bool pop_task_from_other_thread_queue(task_type& task) {
        for (size_t i = 0; i < m_worker_num; ++i) {
            size_t index = (m_index + i + 1) % m_worker_num;
            if (index != m_index && m_queues[index]->try_steal(task)) {
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