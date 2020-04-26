/*
 * StealTaskGroup.cpp
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#include <iostream>
#include "../../Log.h"
#include "StealTaskGroup.h"

namespace hku {

/**
 * “停止”任务
 */
class StopTask : public StealTaskBase {
public:
    StopTask() {}
    virtual ~StopTask() {}

    void run(){};
};

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
        const StealTaskList& taskList = _pTaskGroup->getTaskList();
        size_t total = taskList.size();
        for (size_t i = 0; i < total; i++) {
            taskList[i]->join();
        }

        _pTaskGroup->cancel();
    }

private:
    StealTaskGroup* _pTaskGroup;
};

StealTaskGroup::StealTaskGroup(size_t taskCount, size_t groupSize) {
    _taskList.reserve(taskCount);
    m_runnerNum = (groupSize != 0) ? groupSize : std::thread::hardware_concurrency();
    _runnerList.reserve(m_runnerNum);
    _stopTask = StealTaskPtr(new StopTask());
    _currentRunnerId = 0;
    for (size_t i = 0; i < m_runnerNum; i++) {
        StealTaskRunnerPtr runner(new StealTaskRunner(this, i, _stopTask));
        _runnerList.push_back(runner);
    }

    for (auto i = 0; i < m_runnerNum; i++) {
        m_runner_queues.push_back(std::make_shared<StealRunnerQueue>());
    }

    start();
}

StealTaskGroup::~StealTaskGroup() {}

StealTaskRunnerPtr StealTaskGroup::getRunner(size_t id) {
    StealTaskRunnerPtr result;
    if (id >= m_runnerNum) {
        std::cerr << "[StealTaskGroup::getRunner] Invalid id: " << id << std::endl;
        return result;
    }

    return _runnerList[id];
}

StealTaskRunnerPtr StealTaskGroup::getCurrentRunner() {
    StealTaskRunnerPtr result = _runnerList[_currentRunnerId];
    _currentRunnerId++;
    if (_currentRunnerId >= m_runnerNum) {
        _currentRunnerId = 0;
    }
    return result;
}

StealTaskPtr StealTaskGroup::addTask(const StealTaskPtr& task) {
    if (StealTaskRunner::m_local_queue) {
        HKU_INFO("add task to local queue!");
        StealTaskRunner::m_local_queue->push_front(task);
    }

    _taskList.push_back(task);
    StealTaskRunnerPtr runner = getCurrentRunner();
    task->setTaskRunner(runner.get());
    runner->putTask(task);
    return task;
}

void StealTaskGroup::start() {
    RunnerList::iterator runnerIter;
    for (runnerIter = _runnerList.begin(); runnerIter != _runnerList.end(); runnerIter++) {
        (*runnerIter)->start();
    }
}

void StealTaskGroup::cancel() {
    RunnerList::iterator runnerIter;
    for (runnerIter = _runnerList.begin(); runnerIter != _runnerList.end(); runnerIter++) {
        (*runnerIter)->putTask(_stopTask);
    }
}

void StealTaskGroup::join() {
    RunnerList::iterator runnerIter;
    for (runnerIter = _runnerList.begin(); runnerIter != _runnerList.end(); runnerIter++) {
        (*runnerIter)->join();
    }
}

void StealTaskGroup::stop() {
    StealTaskPtr watchTask(new WatchTask(this));
    StealTaskRunnerPtr runner = getCurrentRunner();
    watchTask->setTaskRunner(runner.get());
    runner->putWatchTask(watchTask);
}

void StealTaskGroup::run() {
    // start();
    stop();
    join();
}

}  // namespace hku
