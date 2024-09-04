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
 * @param worker_num 接收数据后处理时的工作任务组线程数
 * @param addr 服务端地址，为空表示使用 hikyuu 配置文件中的行情服务器地址
 * @ingroup Agent
 */
void HKU_API startSpotAgent(bool print = true,
                            size_t worker_num = std::thread::hardware_concurrency(),
                            const string& addr = string());

/**
 * 终止 Spot 数据接收代理
 * @ingroup Agent
 */
void HKU_API stopSpotAgent();

SpotAgent* getGlobalSpotAgent();

void HKU_API releaseGlobalSpotAgent();

}  // namespace hku