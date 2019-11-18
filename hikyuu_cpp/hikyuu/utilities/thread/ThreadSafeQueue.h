/*
 * ThreadSafeQueue.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-11-17
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_THREAD_THREADSAFEQUEUE_H
#define HIKYUU_UTILITIES_THREAD_THREADSAFEQUEUE_H

#include <queue>
#include <thread>

namespace hku {

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() {}

    void push(T item) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_queue.push(std::move(item));
        m_cond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cond.wait(lk, [this] { return !m_queue.empty(); });
        value = std::move(m_queue.front());
        m_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cond.wait(lk, [this] { return !m_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop();
        return res;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_queue.empty();
    }

private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_cond;
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_THREADSAFEQUEUE_H */
