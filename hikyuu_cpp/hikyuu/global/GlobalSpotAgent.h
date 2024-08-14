/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-21
 *     Author: fasiondog
 */

#pragma once
#include "agent/SpotAgent.h"

namespace hku {

/**
 * 启动 Spot 数据接收代理，如果之前已经处于运行状态，将抛出异常
 * @param print 打印接收数据进展
 * @ingroup Agent
 */
void HKU_API startSpotAgent(bool print = true);

/**
 * 终止 Spot 数据接收代理
 * @ingroup Agent
 */
void HKU_API stopSpotAgent();

SpotAgent* getGlobalSpotAgent();

void releaseGlobalSpotAgent();

}  // namespace hku