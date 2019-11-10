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

class WorkStealQueue {
private:
    typedef FuncWrapper data_type;
    std::deque<data_type> m_queue;
    mutable std::mutex m_mutex;

public:
    WorkStealQueue() {}
    WorkStealQueue(const WorkStealQueue& other) = delete;
    WorkStealQueue& operator=(const WorkStealQueue& other) = delete;

    void push_front(data_type data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_front(std::move(data));
    }

    void push_back(data_type data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_back(std::move(data));
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    bool try_pop(data_type& res) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }

        res = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

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
