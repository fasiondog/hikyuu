/*
 * GlobalTaskGroup.h
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-4-20
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_GLOBAL_TASK_GROUP
#define HKU_GLOBAL_TASK_GROUP

#include "../utilities/thread/StealThreadPool.h"

namespace hku {

/**
 * 获取全局线程池任务组
 * @note 请使用 future 获取任务返回
 */
StealThreadPool* getGlobalTaskGroup();

template <typename ResultType>
using task_handle = std::future<ResultType>;

/**
 * 向全局任务池中增加任务
 */
template <typename FunctionType>
task_handle<typename std::result_of<FunctionType()>::type> addTask(FunctionType f) {
    return getGlobalTaskGroup()->submit(f);
}

/**
 * 程序退出时释放全局任务组实例，仅内部调用
 */
void releaseGlobalTaskGroup();

} /* namespace hku */

#endif /* HKU_GLOBAL_TASK_GROUP */
