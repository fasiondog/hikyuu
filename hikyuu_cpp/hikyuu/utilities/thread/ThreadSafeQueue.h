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
#include <condition_variable>

namespace hku {

/**
 * 线程安全队列
 */
template <typename T>
class ThreadSafeQueue {
public:
    /** 构造函数 */
    ThreadSafeQueue() {}

    /** 将元素插入队列尾部 */
    void push(T item) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_queue.push(std::move(item));
        m_cond.notify_one();
    }

    /** 等待直到从队列头部取出一个元素 */
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cond.wait(lk, [this] { return !m_queue.empty(); });
        value = std::move(m_queue.front());
        m_queue.pop();
    }

    /** 等待直到从队列头部取出一个元素 */
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cond.wait(lk, [this] { return !m_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop();
        return res;
    }

    /** 尝试从队列头部取出一个元素，若成功返回 true, 失败返回 false */
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    /** 尝试从队列头部取出一个元素，若成功返回 true, 失败返回 false */
    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop();
        return res;
    }

    /** 队列是否为空 */
    bool empty() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_queue.empty();
    }

    /** 队列大小，！未加锁，谨慎使用 */
    size_t size() const {
        return m_queue.size();
    }

    /** 清空任务队列 */
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto tmp = std::queue<T>();
        m_queue.swap(tmp);
    }

private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_cond;
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_THREAD_THREADSAFEQUEUE_H */
