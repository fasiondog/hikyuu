/*
 * StealTaskGroup.cpp
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#include <iostream>
#include "../../Log.h"
#include "../exception.h"
#include "StealTaskGroup.h"

namespace hku {

StealTaskGroup::StealTaskGroup(size_t groupSize) : m_steal_index(0), m_done(false) {
    m_runnerNum = (groupSize != 0) ? groupSize : std::thread::hardware_concurrency();
    m_runnerList.reserve(m_runnerNum);

    m_master_queue = std::make_shared<StealMasterQueue>();

    // 创建工作线程队列
    for (auto i = 0; i < m_runnerNum; i++) {
        m_runner_queues.push_back(std::make_shared<StealRunnerQueue>());
    }

    // 创建工作线程
    for (size_t i = 0; i < m_runnerNum; i++) {
        m_runnerList.push_back(std::make_shared<StealTaskRunner>(this, i));
    }

    // 启动各个工作线程
    for (auto runnerIter = m_runnerList.begin(); runnerIter != m_runnerList.end(); ++runnerIter) {
        (*runnerIter)->start();
    }
}

StealTaskGroup::~StealTaskGroup() {
    if (!m_done) {
        join();
    }
}

StealTaskPtr StealTaskGroup::addTask(const StealTaskPtr& task, bool inMain) {
    HKU_CHECK(task, "Input task is nullptr!");
    task->setTaskGroup(this);

    // 如果指定不在主线程且当前线程有效时，插入到当前线性任务队列头部
    // 否则，放入主线程队列
    if (!inMain && StealTaskRunner::m_local_queue) {
        StealTaskRunner::m_local_queue->push_front(task);
    } else {
        m_master_queue->push(task);
    }

    return task;
}

void StealTaskGroup::join() {
    if (m_done) {
        return;
    }

    // 向主任务队列插入“停止”任务
    for (auto i = 0; i < m_runnerNum; i++) {
        auto stopTask = std::make_shared<StopTask>();
        m_master_queue->push(stopTask);
    }

    m_done = true;

    RunnerList::iterator runnerIter;
    for (runnerIter = m_runnerList.begin(); runnerIter != m_runnerList.end(); ++runnerIter) {
        (*runnerIter)->join();
    }
}

void StealTaskGroup::stop() {
    if (m_done) {
        return;
    }

    // 指示各子线程需要停止
    for (auto runnerIter = m_runnerList.begin(); runnerIter != m_runnerList.end(); ++runnerIter) {
        (*runnerIter)->stop();
    }

    // 同时向主任务队列插入任务停止标识，以便唤醒休眠状态的子线程
    for (auto i = 0; i < m_runnerNum; i++) {
        m_master_queue->push(std::make_shared<StopTask>());
    }

    m_done = true;
}

void StealTaskGroup::taskJoinInMaster(const StealTaskPtr& waitingFor) {
    while (waitingFor && !waitingFor->done()) {
        auto task = m_master_queue->try_pop();
        if (task && !task->done()) {
            task->invoke();
        } else {
            stealInMaster(waitingFor);
        }
        std::this_thread::yield();
    }
}

void StealTaskGroup::stealInMaster(const StealTaskPtr& waitingFor) {
    StealTaskPtr task;
    auto total = m_runnerNum;
    for (size_t i = 0; i < total; ++i) {
        if (waitingFor && waitingFor->done()) {
            return;
        }
        size_t index = (m_steal_index + i + 1) % total;
        task = m_runner_queues[index]->try_steal();
        if (task) {
            task->invoke();
            m_steal_index = i;
            return;
        }
    }
}

}  // namespace hku
