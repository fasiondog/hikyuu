/*
 * StealMasterQueue.h
 *
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-4-27
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_TASK_STEAL_MASTER_QUEUE_H
#define HIKYUU_UTILITIES_TASK_STEAL_MASTER_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

#include "StealTaskBase.h"

namespace hku {

/**
 * 主线程任务队列
 * @ingroup TaskGroup
 */
class StealMasterQueue {
public:
    StealMasterQueue() = default;

    /**
     * 将任务放入队列尾部
     * @param item 待执行任务
     */
    void push(const StealTaskPtr& item);

    /**
     * 工作线程等待并弹出头部任务，队列为空时，工作线程阻塞休眠并等待唤醒
     * @return 待执行任务
     */
    StealTaskPtr wait_and_pop();

    /** 尝试从队列头部弹出任务，如果队列为空则弹出空指针，不阻塞线程 */
    StealTaskPtr try_pop();

    /** 队列是否为空 */
    bool empty() const;

private:
    mutable std::mutex m_mutex;
    std::queue<StealTaskPtr> m_queue;
    std::condition_variable m_cond;
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_TASK_STEAL_MASTER_QUEUE_H */
