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
#include "StealTaskRunner.h"
#include "StealTaskGroup.h"

#define QUEUE_LOCK std::lock_guard<std::mutex> lock(_mutex);

namespace hku {

StealTaskRunner::StealTaskRunner(StealTaskGroup* group, size_t id, StealTaskPtr stopTask) {
    _id = id;
    _group = group;
    _stopTask = stopTask;
}

StealTaskRunner::~StealTaskRunner() {}

/**
 * 加入一个普通任务，将其放入私有队列的后端
 * @param task
 */
void StealTaskRunner::putTask(const StealTaskPtr& task) {
    QUEUE_LOCK;
    _queue.push_back(task);
}

/**
 * 加入一个特殊的监护任务，在所有任务完成后，终止任务包的执行
 * @param task - 必须是预定义的监护任务，该任务在所有任务完成后，终止任务包
 */
void StealTaskRunner::putWatchTask(const StealTaskPtr& task) {
    QUEUE_LOCK;
    _queue.push_front(task);
}

/**
 * 从自己的任务队列中提取一个任务，供自己执行
 * @return StealTaskPtr - 提取的任务
 */
StealTaskPtr StealTaskRunner::takeTaskBySelf() {
    QUEUE_LOCK;
    StealTaskPtr result;
    if (!_queue.empty()) {
        result = _queue.back();
        _queue.pop_back();
    }

    return result;
}

/**
 * 从自己的任务队列中提取一个任务，供其他线程使用
 * @return StealTaskPtr - 提取的任务
 */
StealTaskPtr StealTaskRunner::takeTaskByOther() {
    QUEUE_LOCK;
    StealTaskPtr result;
    if (!_queue.empty()) {
        StealTaskPtr front = _queue.front();
        //如果提取的任务是停止任务，则放弃并返回空
        if (front != _stopTask) {
            result = front;
            _queue.pop_front();
        }
    }

    return result;
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
    m_thread.join();
}

/**
 * 循环执行所有分配的任务
 */
void StealTaskRunner::run() {
    StealTaskPtr task;
    try {
        while (task != _stopTask) {
            task = takeTaskBySelf();
            if (task) {
                if (!task->isDone()) {
                    task->invoke();
                }

            } else {
                steal(StealTaskPtr());
            }
        }

    } catch (...) {
        std::cerr << "[TaskRunner::run] Some error!" << std::endl;
    }
}

/**
 * 等待某一任务执行结束
 * @param waitingFor - 等待结束的任务
 */
void StealTaskRunner::taskJoin(const StealTaskPtr& waitingFor) {
    while (!waitingFor->isDone()) {
        StealTaskPtr task = takeTaskBySelf();
        if (task) {
            if (!task->isDone()) {
                task->invoke();
            }
        } else {
            steal(waitingFor);
        }
    }
}

void StealTaskRunner::steal(const StealTaskPtr& waitingFor) {
    StealTaskPtr task;

#ifdef _WIN32
    std::srand((unsigned)time(NULL));
#else
    struct timeval tstart;
    struct timezone tz;
    gettimeofday(&tstart, &tz);
    size_t temp = tstart.tv_usec;
    std::srand(temp);
#endif

    size_t total = _group->size();
    size_t ran_num = std::rand() % total;
    for (size_t i = 0; i < total; i++) {
        StealTaskRunnerPtr tr = _group->getRunner(ran_num);
        if (waitingFor && waitingFor->isDone()) {
            break;
        }

        if (tr && tr.get() != this) {
            task = tr->takeTaskByOther();
            if (task) {
                break;
            }
            std::this_thread::yield();
            ran_num = (ran_num + 1) % total;
        }
    }

    if (task && !task->isDone()) {
        task->invoke();
    }
}

}  // namespace hku
