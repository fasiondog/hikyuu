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

namespace hku {

class StealTaskRunner;
class StealTaskGroup;

/**
 * 并行任务基类
 * @ingroup TaskGroup
 */
class HKU_API StealTaskBase : public std::enable_shared_from_this<StealTaskBase> {
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

    // StealTaskGroup 设置
    void setTaskRunner(StealTaskRunner *runner) {
        m_runner = runner;
    }

    void setTaskGroup(StealTaskGroup *group) {
        m_group = group;
    }

private:
    mutable bool m_done;  // 标记该任务是否已执行完毕
    mutable StealTaskRunner *m_runner;
    StealTaskGroup *m_group;  // 任务组指针
};

typedef std::shared_ptr<StealTaskBase> StealTaskPtr;
typedef std::vector<StealTaskPtr> StealTaskList;
typedef std::shared_ptr<StealTaskList> StealTaskListPtr;

typedef StealTaskBase TaskBase;
typedef StealTaskPtr TaskPtr;
typedef StealTaskList TaskList;
typedef StealTaskListPtr TaskListPtr;

}  // namespace hku

#endif /* STEALTASKBASE_H_ */
