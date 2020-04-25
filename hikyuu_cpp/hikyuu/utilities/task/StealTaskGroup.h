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

namespace hku {

/**
 * 偷取式并行任务组
 * @ingroup TaskGroup
 */
class HKU_API StealTaskGroup {
public:
    /**
     * 构造函数
     * @param taskCount - 预计的任务总数
     * @param groupSize - 线程数量： 0表示自动选择2倍CPU数量
     * @return
     */
    StealTaskGroup(size_t taskCount = 3072, size_t groupSize = 0);
    virtual ~StealTaskGroup();

    size_t size() const {
        return _groupSize;
    }
    StealTaskRunnerPtr getRunner(size_t id);
    StealTaskRunnerPtr getCurrentRunner();

    //增加一个任务
    void addTask(const StealTaskPtr& task);

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
    size_t _groupSize;
    StealTaskPtr _stopTask;
    size_t _currentRunnerId;  //记录当前执行addTask任务时，需放入的TaskRunnerid，用于均衡任务分配
};

typedef std::shared_ptr<StealTaskGroup> StealTaskGroupPtr;

typedef StealTaskGroup TaskGroup;
typedef StealTaskGroupPtr TaskGroupPtr;

}  // namespace hku

#endif /* STEALTASKGROUP_H_ */
