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
#include <shared_mutex>
#include "FuncWrapper.h"

namespace hku {

/**
 * Task stealing queue
 */
class WorkStealQueue {
private:
    typedef FuncWrapper data_type;
    std::deque<data_type> m_queue;
    mutable std::shared_mutex m_mutex;

public:
    /** Constructor */
    WorkStealQueue() {}

    // The copy constructor and the assignment overload are disabled
    WorkStealQueue(const WorkStealQueue& other) = delete;
    WorkStealQueue& operator=(const WorkStealQueue& other) = delete;

    /** Insert the data into the head of the queue */
    void push_front(data_type&& data) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_queue.push_front(std::move(data));
    }

    /** Insert the data into the tail of the queue */
    void push_back(data_type&& data) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_queue.push_back(std::move(data));
    }

    /** Whether the queue is empty */
    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_queue.empty();
    }

    /** Queue size */
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_queue.size();
    }

    void clear() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto tmp = std::deque<data_type>();
        m_queue.swap(tmp);
    }

    /**
     * Try to pop a piece of data from the head of the queue
     * @param res stores the popped data
     * @return false is returned if the queue was originally empty, otherwise true
     */
    bool try_pop(data_type& res) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }

        res = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    /**
     * Try to steal a piece of data from the tail of the queue
     * @param res stores the stolen data
     * @return false is returned if the queue was originally empty, otherwise true
     */
    bool try_steal(data_type& res) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }

        if (m_queue.back().isNullTask()) {
            return false;
        }

        res = std::move(m_queue.back());
        m_queue.pop_back();
        return true;
    }
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_WORKSTEALQUEUE_H */
