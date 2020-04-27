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
#include "StealRunnerQueue.h"

namespace hku {

class StealTaskGroup;

/**
 * 偷取式并行任务组内部执行引擎
 * @ingroup TaskGroup
 */
class StealTaskRunner {
    friend class StealTaskGroup;
    friend class StealTaskBase;

public:
    StealTaskRunner(StealTaskGroup* group, size_t id, StealTaskPtr stopTask);
    virtual ~StealTaskRunner();

private:
    /**
     * 加入一个普通任务，将其放入私有队列的后端
     * @param task
     */
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
        return m_group;
    }

    std::thread::id get_thread_id() const {
        return m_thread_id;
    }

private:
    size_t m_index;           // 表示在任务组中的第几个线程
    StealTaskGroup* m_group;  // 所属任务组的指针
    StealTaskPtr _stopTask;

    inline static thread_local StealRunnerQueue* m_local_queue = nullptr;  //本地任务队列
    inline static thread_local size_t m_local_index = 0;        // 在任务组中的序号(m_index)
    inline static thread_local bool m_locla_need_stop = false;  // 线程停止运行指示

    std::thread m_thread;         // 本地工作线程
    std::thread::id m_thread_id;  // 线程id

    // 线程内工作任务队列
    std::mutex m_queue_mutex;
    typedef std::deque<StealTaskPtr> Queue;
    Queue m_queue;
};

typedef std::shared_ptr<StealTaskRunner> StealTaskRunnerPtr;

}  // namespace hku

#endif /* STEALTASKRUNNER_H_ */
