/*
 * StealTaskBase.h
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#pragma once
#ifndef STEALTASKBASE_H_
#define STEALTASKBASE_H_

#ifndef HKU_API
#define HKU_API
#endif

#include <thread>
#include <memory>
#include <hikyuu/Log.h>

namespace hku {

class StealTaskRunner;
class StealTaskGroup;

/**
 * 并行任务基类
 * @ingroup TaskGroup
 */
class StealTaskBase : public std::enable_shared_from_this<StealTaskBase> {
    friend class StealTaskRunner;
    friend class StealTaskGroup;

public:
    StealTaskBase();
    virtual ~StealTaskBase();

    /**
     * 任务的实际执行动作，子类任务必须实现该接口
     */
    virtual void run();

    /**
     * 任务是否已完成
     */
    bool done() const {
        return m_done;
    }

    /**
     * 等待任务完成
     */
    void join();

private:
    // StealTaskRunner 实际执行任务
    void invoke();

    void setTaskGroup(StealTaskGroup *group) {
        m_group = group;
    }

private:
    mutable bool m_done;      // 标记该任务是否已执行完毕
    StealTaskGroup *m_group;  // 任务组指针
};

typedef std::shared_ptr<StealTaskBase> StealTaskPtr;
typedef std::vector<StealTaskPtr> StealTaskList;
typedef std::shared_ptr<StealTaskList> StealTaskListPtr;

typedef StealTaskBase TaskBase;
typedef StealTaskPtr TaskPtr;
typedef StealTaskList TaskList;
typedef StealTaskListPtr TaskListPtr;

/**
 * “停止”任务，用于指示工作线程停止运行
 */
class StopTask : public StealTaskBase {
public:
    StopTask() {}
    virtual ~StopTask() {}

    void run() {
        HKU_INFO("stop task has runned!");
    };
};

}  // namespace hku

#endif /* STEALTASKBASE_H_ */
