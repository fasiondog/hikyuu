/*
 * StealRunnerQueue.h
 *
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-4-27
 *      Author: fasiondog
 */

#include "StealRunnerQueue.h"

namespace hku {

/* 将数据插入队列头部 */
void StealRunnerQueue::push_front(const StealTaskPtr& task) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_front(task);
}

/* 将数据插入队列尾部 */
void StealRunnerQueue::push_back(const StealTaskPtr& task) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(task);
}

/* 队列是否为空 */
bool StealRunnerQueue::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

/* 尝试从队列头部弹出一条数数据, 如果失败返回空指针 */
StealTaskPtr StealRunnerQueue::try_pop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    StealTaskPtr result;
    if (m_queue.empty()) {
        return result;
    }

    result = m_queue.front();
    m_queue.pop_front();
    return result;
}

/* 尝试从队列尾部偷取一条数据，失败返回空指针 */
StealTaskPtr StealRunnerQueue::try_steal() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return StealTaskPtr();
    }

    StealTaskPtr result = m_queue.back();
    if (typeid(*result) == typeid(StopTask)) {
        return StealTaskPtr();
    }

    m_queue.pop_back();
    return result;
}

} /* namespace hku */