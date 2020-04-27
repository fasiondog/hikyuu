/*
 * StealMasterQueue.cpp
 *
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-4-27
 *      Author: fasiondog
 */

#include "StealMasterQueue.h"

namespace hku {

// 将任务放入队列尾部
void StealMasterQueue::push(const StealTaskPtr& item) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_queue.push(item);
    m_cond.notify_one();
}

// 工作线程等待并弹出头部任务，队列为空时，工作线程阻塞休眠并等待唤醒
StealTaskPtr StealMasterQueue::wait_and_pop() {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cond.wait(lk, [this] { return !m_queue.empty(); });
    auto result = m_queue.front();
    m_queue.pop();
    return result;
}

// 尝试从队列头部弹出任务，如果队列为空则弹出空指针，不阻塞线程
StealTaskPtr StealMasterQueue::try_pop() {
    std::lock_guard<std::mutex> lk(m_mutex);
    StealTaskPtr result;
    if (m_queue.empty()) {
        return result;
    }
    result = m_queue.front();
    m_queue.pop();
    return result;
}

// 队列是否为空
bool StealMasterQueue::empty() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_queue.empty();
}

}  // namespace hku