/*
 * StealTaskGroup.h
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILITIES_STEALTASKGROUP_H_
#define HKU_UTILITIES_STEALTASKGROUP_H_

#include "StealMasterQueue.h"
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
     * @param groupSize - 线程数量： 0表示自动选择同CPU数量的线程数
     * @return
     */
    StealTaskGroup(size_t groupSize = 0);

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

    //增加一个任务
    StealTaskPtr addTask(const StealTaskPtr& task, bool inMain = true);

    //所有任务都已加入指示
    void stop();

    //强制终止
    // void cancel();

    //等待执行结束
    void join();

    bool done() const {
        return m_done;
    }

    void taskJoinInMaster(const StealTaskPtr& waitingFor);
    void stealInMaster(const StealTaskPtr& waitingFor);

private:
    typedef std::vector<StealTaskRunnerPtr> RunnerList;
    RunnerList m_runnerList;
    size_t m_runnerNum;
    size_t m_steal_index;  // 辅助记录主线程从哪个子线程偷取任务
    bool m_done;           // 任务组执行结束标志

    std::shared_ptr<StealMasterQueue> m_master_queue;                // 主任务队列
    std::vector<std::shared_ptr<StealRunnerQueue>> m_runner_queues;  // 任务队列（每个工作线程一个）
};

typedef std::shared_ptr<StealTaskGroup> StealTaskGroupPtr;

typedef StealTaskGroup TaskGroup;
typedef StealTaskGroupPtr TaskGroupPtr;

}  // namespace hku

#endif /* HKU_UTILITIES_STEALTASKGROUP_H_ */
