/*
 * MQStealQueue.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-11-17
 *      Author: fasiondog
 */

#pragma once

#include <queue>
#include <thread>
#include <condition_variable>

namespace hku {

template <typename T>
class MQStealQueue {
public:
    MQStealQueue() {}

    void push(T item) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_queue.push_back(std::move(item));
        m_cond.notify_one();
    }

    /** 将数据插入队列头部 */
    void push_front(T data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_front(std::move(data));
        m_cond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cond.wait(lk, [this] { return !m_queue.empty(); });
        value = std::move(m_queue.front());
        m_queue.pop_front();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cond.wait(lk, [this] { return !m_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop_front();
        return res;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop_front();
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

    /**
     * 尝试从队列尾部偷取一条数据
     * @param res 存储偷取的数据
     * @return 如果原本队列为空返回 false，否则为 true
     */
    bool try_steal(T& res) {
        std::lock_guard<std::mutex> lock(m_mutex);
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

    bool empty() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_queue.empty();
    }

    // 队列大小，无锁
    size_t size() const {
        return m_queue.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto tmp = std::deque<T>();
        m_queue.swap(tmp);
    }

private:
    mutable std::mutex m_mutex;
    std::deque<T> m_queue;
    std::condition_variable m_cond;
};

} /* namespace hku */

