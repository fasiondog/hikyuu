/*
 * StealRunnerQueue.h
 *
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-4-26
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_TASK_STEAL_RUNNER_QUEUE_H
#define HIKYUU_UTILITIES_TASK_STEAL_RUNNER_QUEUE_H

#include <deque>
#include <mutex>
#include "StealTaskBase.h"

namespace hku {

/**
 * 工作线程任务队列
 * @ingroup TaskGroup
 */
class StealRunnerQueue {
public:
    /** 构造函数 */
    StealRunnerQueue() = default;
    ~StealRunnerQueue() = default;

    // 禁用赋值构造和赋值重载
    StealRunnerQueue(const StealRunnerQueue& other) = delete;
    StealRunnerQueue& operator=(const StealRunnerQueue& other) = delete;

    /**
     * 将任务插入队列头部
     * @param task 任务
     */
    void push_front(const StealTaskPtr& task);

    /**
     * 将任务插入队列尾部
     * @param task 任务
     */
    void push_back(const StealTaskPtr& task);

    /** 队列是否为空 */
    bool empty() const;

    /** 队列当前大小 */
    size_t size() const {
        return m_queue.size();
    }

    /**
     * 尝试从队列头部弹出一条数数据，失败返回空指针
     * @param res 存储弹出的数据
     * @return 如果原本队列为空指针
     */
    StealTaskPtr try_pop();

    /**
     * 尝试从队列尾部偷取一条数据，失败返回空指针
     * @param res 存储偷取的数据
     * @return 返回任务指针或空指针
     */
    StealTaskPtr try_steal();

private:
    std::deque<StealTaskPtr> m_queue;
    mutable std::mutex m_mutex;
};

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_TASK_STEAL_RUNNER_QUEUE_H */
