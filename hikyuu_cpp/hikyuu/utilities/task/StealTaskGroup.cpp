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

/**
 * 监护任务，在所有任务完成后，向所有线程发出StopTask
 */
class WatchTask : public StealTaskBase {
public:
    WatchTask(StealTaskGroup* pTaskGroup) {
        _pTaskGroup = pTaskGroup;
    }
    virtual ~WatchTask() {}

    void run() {
        /*const StealTaskList& taskList = _pTaskGroup->getTaskList();
        size_t total = taskList.size();
        for (size_t i = 0; i < total; i++) {
            taskList[i]->join();
        }*/

        //_pTaskGroup->cancel();
    }

private:
    StealTaskGroup* _pTaskGroup;
};

StealTaskGroup::StealTaskGroup(size_t groupSize) : m_currentRunnerId(0), m_done(false) {
    m_runnerNum = (groupSize != 0) ? groupSize : std::thread::hardware_concurrency();
    m_runnerList.reserve(m_runnerNum);

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

StealTaskRunnerPtr StealTaskGroup::getCurrentRunner() {
    StealTaskRunnerPtr result = m_runnerList[m_currentRunnerId];
    m_currentRunnerId++;
    if (m_currentRunnerId >= m_runnerNum) {
        m_currentRunnerId = 0;
    }
    return result;
}

StealTaskPtr StealTaskGroup::addTask(const StealTaskPtr& task) {
    HKU_CHECK(task, "Input task is nullptr!");
    task->setTaskGroup(this);

    if (StealTaskRunner::m_local_queue) {
        // 如果是在子线程中执行增加任务，则直接插入子任务队列的头部
        StealTaskRunner::m_local_queue->push_front(task);

    } else {
        // 如果是在主线程中增加任务，则轮流插入主线程和各子线程任务队列的尾部
        if (m_currentRunnerId >= m_runnerNum) {
            m_master_queue.push(task);
        } else {
            m_runner_queues[m_currentRunnerId]->push_back(task);
        }
        m_currentRunnerId++;
        if (m_currentRunnerId > m_runnerNum) {
            m_currentRunnerId = 0;
        }
    }

    return task;
}

void StealTaskGroup::join() {
    if (m_done) {
        return;
    }

    // 向主任务队列插入“停止”任务
    std::vector<StealTaskPtr> stopTaskList;
    for (auto i = 0; i < m_runnerNum; i++) {
        auto stopTask = std::make_shared<StopTask>();
        m_master_queue.push(stopTask);
    }

    // 等待“停止”任务被执行
    for (auto& task : stopTaskList) {
        task->join();
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

    m_done = true;

    // 指示各子线程需要停止
    for (auto runnerIter = m_runnerList.begin(); runnerIter != m_runnerList.end(); ++runnerIter) {
        (*runnerIter)->stop();
    }

    // 同时向主任务队列插入任务停止标识，以便唤醒休眠状态的子线程
    for (auto i = 0; i < m_runnerNum; i++) {
        m_master_queue.push(std::make_shared<StopTask>());
    }
}

void StealTaskGroup::taskJoinInMaster(const StealTaskPtr& waitingFor) {
    while (!waitingFor->done()) {
        auto task = m_master_queue.try_pop();
        if (task && !task->done()) {
            task->invoke();
        } else {
            // steal(waitingFor);
        }
    }
}

void StealTaskGroup::stealInMaster(const StealTaskPtr& waitingFor) {
    StealTaskPtr task;

#ifdef _WIN32
    std::srand((unsigned)time(NULL));
#else
    struct timeval tstart;
    struct timezone tz;
    gettimeofday(&tstart, &tz);
    size_t temp = tstart.tv_usec;
    std::srand(temp);
#endif

    size_t total = m_runnerNum;
    size_t ran_num = std::rand() % total;
    for (size_t i = 0; i < total; i++) {
        if (waitingFor && waitingFor->done()) {
            break;
        }

        task = m_runner_queues[ran_num]->try_steal();
        if (task) {
            break;
        }

        std::this_thread::yield();
        ran_num = (ran_num + 1) % total;
    }

    if (task && !task->done()) {
        task->invoke();
    }
}

}  // namespace hku
