/*
 * WorkStealQueue.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_THREAD_WORKSTEALQUEUE_H
#define HIKYUU_UTILITIES_THREAD_WORKSTEALQUEUE_H

#include <deque>
#include <mutex>
#include "FuncWrapper.h"

namespace hku {

/**
 * 任务偷取队列
 */
class WorkStealQueue {
private:
    typedef FuncWrapper data_type;
    std::deque<data_type> m_queue;
    mutable std::mutex m_mutex;

public:
    /** 构造函数 */
    WorkStealQueue() {}

    // 禁用赋值构造和赋值重载
    WorkStealQueue(const WorkStealQueue& other) = delete;
    WorkStealQueue& operator=(const WorkStealQueue& other) = delete;

    /** 将数据插入队列头部 */
    void push_front(data_type data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_front(std::move(data));
    }

    /** 将数据插入队列尾部 */
    void push_back(data_type data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_back(std::move(data));
    }

    /** 队列是否为空 */
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    /** 队列大小，！未加锁，谨慎使用 */
    size_t size() const {
        return m_queue.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto tmp = std::deque<data_type>();
        m_queue.swap(tmp);
    }

    /**
     * 尝试从队列头部弹出一条数数据
     * @param res 存储弹出的数据
     * @return 如果原本队列为空返回 false，否则为 true
     */
    bool try_pop(data_type& res) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }

        res = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    /**
     * 尝试从队列尾部偷取一条数据
     * @param res 存储偷取的数据
     * @return 如果原本队列为空返回 false，否则为 true
     */
    bool try_steal(data_type& res) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }

        res = std::move(m_queue.back());
        m_queue.pop_back();
        return true;
    }
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_WORKSTEALQUEUE_H */
