/*
 * StealTaskRunner.h
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#pragma once
#ifndef STEALTASKRUNNER_H_
#define STEALTASKRUNNER_H_

#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
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
    /**
     * 子工作线程构造函数
     * @param group 所属任务组指针
     * @param id 所属任务组中的id
     */
    StealTaskRunner(StealTaskGroup* group, size_t id);

    virtual ~StealTaskRunner();

private:
    StealTaskPtr takeTaskFromLocal();          // 从自己的任务队列中获取任务
    StealTaskPtr takeTaskFromMasterAndWait();  // 等待直至从主线程队列中获取任务
    StealTaskPtr takeTaskFromOther();          // 从其他线程队列偷取任务

    void start();
    void join();
    void run();
    void taskJoin(const StealTaskPtr& waitingFor);

    void stop();

    StealTaskGroup* getTaskRunnerGroup() {
        return m_group;
    }

    std::thread::id get_thread_id() const {
        return m_thread_id;
    }

private:
    size_t m_index;           // 表示在任务组中的第几个线程
    StealTaskGroup* m_group;  // 所属任务组的指针

    inline static thread_local StealRunnerQueue* m_local_queue = nullptr;  //本地任务队列
    inline static thread_local StealTaskGroup* m_local_group = nullptr;    //任务组指针
    inline static thread_local StealTaskRunner* m_local_runner = nullptr;  // 记录自身runner指针
    inline static thread_local size_t m_local_index = 0;        // 在任务组中的序号(m_index)
    inline static thread_local bool m_locla_need_stop = false;  // 线程停止运行指示

    std::atomic_bool m_done;      //// 线程终止指示
    std::thread m_thread;         // 本地工作线程
    std::thread::id m_thread_id;  // 线程id
};

typedef std::shared_ptr<StealTaskRunner> StealTaskRunnerPtr;

}  // namespace hku

#endif /* STEALTASKRUNNER_H_ */
