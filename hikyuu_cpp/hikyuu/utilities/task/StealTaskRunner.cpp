/*
 * StealTaskRunner.cpp
 *
 *  Created on: 2010-3-19
 *      Author: fasiondog
 */

#ifdef _WIN32
#include <ctime>
#endif

#include <functional>
#include <iostream>
#include "../../Log.h"
#include "StealTaskRunner.h"
#include "StealTaskGroup.h"

#define QUEUE_LOCK std::lock_guard<std::mutex> lock(m_queue_mutex);

namespace hku {

StealTaskRunner::StealTaskRunner(StealTaskGroup* group, size_t id) : m_done(false) {
    m_index = id;
    m_group = group;
}

StealTaskRunner::~StealTaskRunner() {}

// 尝试从自己的任务队列中提取一个任务，供自己执行
StealTaskPtr StealTaskRunner::takeTaskFromLocal() {
    return m_local_queue->try_pop();
}

// 阻塞等待直至从主线程任务队列中获取到任务
StealTaskPtr StealTaskRunner::takeTaskFromMasterAndWait() {
    return m_local_group->m_master_queue->wait_and_pop();
}

// 尝试从其他子线程任务队列中偷取任务
StealTaskPtr StealTaskRunner::takeTaskFromOther() {
    StealTaskPtr task;
    auto total = m_local_group->m_runnerNum;
    for (size_t i = 0; i < total; ++i) {
        size_t index = (m_local_index + i + 1) % total;
        task = m_local_group->m_runner_queues[index]->try_steal();
        if (task) {
            return task;
        }
    }
    return task;
}

/**
 * 启动内部线程，执行分配的任务
 */
void StealTaskRunner::start() {
    m_thread = std::thread(std::bind(&StealTaskRunner::run, this));
}

/**
 * 等待内部线程终止
 */
void StealTaskRunner::join() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void StealTaskRunner::stop() {
    m_done = true;
}

/**
 * 循环执行所有分配的任务，线程函数
 */
void StealTaskRunner::run() {
    m_thread_id = std::this_thread::get_id();
    m_local_group = m_group;
    m_local_runner = this;
    m_local_queue = m_local_group->m_runner_queues[m_index].get();
    m_local_index = m_index;
    m_locla_need_stop = false;
    StealTaskPtr task;
    try {
        while (!m_done && (!task || typeid(*task) != typeid(StopTask))) {
            // 从本地队列中获取待执行任务
            task = takeTaskFromLocal();

            // 如果本地队列中没有取到任务，则尝试从其他子线程队列中偷取任务
            if (!task) {
                task = takeTaskFromOther();
            }

            // 如果本地和其他子线程任务队列中都无法获取任务，则等待并直至从主任务队列中获取任务
            if (!task) {
                task = takeTaskFromMasterAndWait();
            }

            if (task && !task->done()) {
                task->invoke();
            }
            std::this_thread::yield();
        }
    } catch (std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR("Unknown error!");
    }
}

/**
 * 在当前子线程中等待某一任务执行结束，如等待的任务未结束，则先执行其他任务
 * @param waitingFor - 等待结束的任务
 */
void StealTaskRunner::taskJoin(const StealTaskPtr& waitingFor) {
    while (waitingFor && !waitingFor->done()) {
        // 如果获取的任务有效且任务未执行，则执行该任务;
        // 否则从其他子线程任务队列中进行偷取
        auto task = takeTaskFromLocal();
        if (!task) {
            task = takeTaskFromOther();
        }

        if (task && !task->done()) {
            task->invoke();
        }

        std::this_thread::yield();
    }
}

}  // namespace hku
