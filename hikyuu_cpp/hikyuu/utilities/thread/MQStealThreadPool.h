/*
 * StealMQStealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once

//#include <fmt/format.h>
#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include "FuncWrapper.h"
#include "MQStealQueue.h"

namespace hku {

/**
 * @brief 普通多任务队列线程池，任务之间彼此独立不能互相等待
 * @note 任务运行之间如存在先后顺序，请使用 StealThreadPool。
 * @details
 * @ingroup MQStealThreadPool
 */
class MQStealThreadPool {
public:
    /**
     * 默认构造函数，创建和当前系统CPU数一致的线程数
     */
    MQStealThreadPool() : MQStealThreadPool(std::thread::hardware_concurrency(), true) {}

    /**
     * 构造函数，创建指定数量的线程
     * @param n 指定的线程数
     */
    explicit MQStealThreadPool(size_t n, bool util_empty = true)
    : m_done(false), m_init_finished(false), m_worker_num(n), m_runnging_util_empty(util_empty) {
        try {
            for (int i = 0; i < m_worker_num; i++) {
                // 创建工作线程及其任务队列
                m_threads_status.push_back(nullptr);
                m_queues.push_back(
                  std::unique_ptr<MQStealQueue<task_type>>(new MQStealQueue<task_type>));
                m_threads.push_back(std::thread(&MQStealThreadPool::worker_thread, this, i));
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
            throw std::logic_error("Can't submit a task to the stopped MQStealThreadPool!");
        }

        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        task_handle<result_type> res(task.get_future());
        if (m_local_work_queue) {
            // 本地线程任务从前部入队列（递归成栈）
            m_local_work_queue->push_front(std::move(task));
            return res;
        }

        size_t min_count = std::numeric_limits<size_t>::max();
        int index = -1;
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
            if (m_threads_status[i]) {
                m_threads_status[i]->store(true);
            }
            m_queues[i]->push(FuncWrapper());
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
            for (size_t i = 0; i < m_worker_num; i++) {
                while (m_queues[i]->size() != 0) {
                    std::this_thread::yield();
                }
                if (m_threads_status[i]) {
                    m_threads_status[i]->store(true);
                }
            }
            m_done = true;
        }

        for (size_t i = 0; i < m_worker_num; i++) {
            m_queues[i]->push(std::move(FuncWrapper()));
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
    bool m_init_finished;        // 线程池是否初始化完毕
    size_t m_worker_num;         // 工作线程数量
    bool m_runnging_util_empty;  // 运行直到队列空时停止

    std::vector<std::atomic_bool*> m_threads_status;                 // 工作线程状态
    std::vector<std::unique_ptr<MQStealQueue<task_type>>> m_queues;  // 线程任务队列
    std::vector<std::thread> m_threads;                              // 工作线程

    // 线程本地变量
    inline static thread_local MQStealQueue<task_type>* m_local_work_queue =
      nullptr;                                    // 本地任务队列
    inline static thread_local int m_index = -1;  //在线程池中的序号
    inline static thread_local std::atomic_bool m_thread_need_stop = false;  // 线程停止运行指示

    void worker_thread(int index) {
        m_thread_need_stop = false;
        m_index = index;
        m_threads_status[index] = &m_thread_need_stop;
        m_local_work_queue = m_queues[m_index].get();
        while (!m_done && !m_thread_need_stop) {
            run_pending_task();
            std::this_thread::yield();
        }
        // fmt::print("thread ({}) finished!\n", std::this_thread::get_id());
    }

    void run_pending_task() {
        task_type task;
        if (m_local_work_queue->try_pop(task)) {
            if (task.isNullTask()) {
                m_thread_need_stop = true;
            } else {
                task();
                std::this_thread::yield();
            }
            return;
        }

        if (pop_task_from_other_thread_queue(task)) {
            task();
            std::this_thread::yield();
            return;
        }

        m_local_work_queue->wait_and_pop(task);
        if (task.isNullTask()) {
            m_thread_need_stop = true;
        } else {
            task();
            std::this_thread::yield();
        }
    }

    bool pop_task_from_other_thread_queue(task_type& task) {
        // 线程池尚未初始化化完成时，其他任务队列可能尚未创建
        // 此时不能从其他队列偷取任务
        if (!m_init_finished) {
            return false;
        }
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
