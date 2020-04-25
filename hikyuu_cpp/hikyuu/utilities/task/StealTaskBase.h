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

/**
 * 并行任务基类
 * @ingroup TaskGroup
 */
class HKU_API StealTaskBase : public std::enable_shared_from_this<StealTaskBase> {
public:
    StealTaskBase();
    virtual ~StealTaskBase();

    virtual void run() = 0;

    bool isDone() const {
        return _done;
    }

    void fork(StealTaskRunner *);
    void join();
    void invoke();

    void setTaskRunner(StealTaskRunner *runner) {
        _runner = runner;
    }
    StealTaskRunner *getTaskRunner() {
        return _runner;
    }

private:
    mutable bool _done;
    mutable StealTaskRunner *_runner;
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
