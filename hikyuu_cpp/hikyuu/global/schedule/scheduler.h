/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#pragma once

#include "../../utilities/TimerManager.h"

namespace hku {

/**
 * 获取全局调度实例指针
 */
TimerManager *getScheduler();

/**
 * 用于程序退出时释放全局调度实例，仅内部使用
 */
void releaseScheduler();

}  // namespace hku