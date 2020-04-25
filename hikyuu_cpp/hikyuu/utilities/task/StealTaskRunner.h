/*
 * StealTaskRunner.h
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#pragma once
#ifndef STEALTASKRUNNER_H_
#define STEALTASKRUNNER_H_

#include <mutex>
#include <deque>
#include <list>
#include "StealTaskBase.h"

namespace hku {

class StealTaskGroup;

/**
 * 偷取式并行任务组内部执行引擎
 * @ingroup TaskGroup
 */
class HKU_API StealTaskRunner {
public:
    StealTaskRunner(StealTaskGroup* group, size_t id, StealTaskPtr stopTask);
    virtual ~StealTaskRunner();

    void putTask(const StealTaskPtr&);
    void putWatchTask(const StealTaskPtr&);

    StealTaskPtr takeTaskBySelf();
    StealTaskPtr takeTaskByOther();

    void start();
    void join();
    void run();
    void steal(const StealTaskPtr& waitingFor);
    void taskJoin(const StealTaskPtr& waitingFor);

    StealTaskGroup* getTaskRunnerGroup() {
        return _group;
    }

private:
    size_t _id;
    StealTaskGroup* _group;
    StealTaskPtr _stopTask;

    std::thread m_thread;
    std::mutex _mutex;

    typedef std::list<StealTaskPtr> Queue;
    Queue _queue;
};

typedef std::shared_ptr<StealTaskRunner> StealTaskRunnerPtr;

}  // namespace hku

#endif /* STEALTASKRUNNER_H_ */
