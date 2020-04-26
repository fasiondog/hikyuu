/*
 * StealTaskGroup.h
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#pragma once
#ifndef STEALTASKGROUP_H_
#define STEALTASKGROUP_H_

#include "StealTaskRunner.h"
#include "StealRunnerQueue.h"

namespace hku {

/**
 * 偷取式并行任务组
 * @ingroup TaskGroup
 */
class HKU_API StealTaskGroup {
    friend class StealTaskRunner;

public:
    /**
     * 构造函数
     * @param taskCount - 预计的任务总数
     * @param groupSize - 线程数量： 0表示自动选择2倍CPU数量
     * @return
     */
    StealTaskGroup(size_t taskCount = 3072, size_t groupSize = 0);

    /**
     * 析构函数
     */
    virtual ~StealTaskGroup();

    /**
     * 工作线程数量
     */
    size_t size() const {
        return m_runnerNum;
    }

    StealTaskRunnerPtr getRunner(size_t id);
    StealTaskRunnerPtr getCurrentRunner();

    //增加一个任务
    StealTaskPtr addTask(const StealTaskPtr& task);

    void start();

    //所有任务都已加入指示
    void stop();

    //强制终止
    void cancel();

    //等待执行结束
    void join();

    void run();

    const StealTaskList& getTaskList() const {
        return _taskList;
    }

private:
    typedef std::vector<StealTaskRunnerPtr> RunnerList;
    RunnerList _runnerList;
    StealTaskList _taskList;
    size_t m_runnerNum;
    StealTaskPtr _stopTask;
    size_t _currentRunnerId;  //记录当前执行addTask任务时，需放入的TaskRunnerid，用于均衡任务分配

    std::vector<std::shared_ptr<StealRunnerQueue>> m_runner_queues;  // 任务队列（每个工作线程一个）
};

typedef std::shared_ptr<StealTaskGroup> StealTaskGroupPtr;

typedef StealTaskGroup TaskGroup;
typedef StealTaskGroupPtr TaskGroupPtr;

}  // namespace hku

#endif /* STEALTASKGROUP_H_ */
