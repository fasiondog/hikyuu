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
 * Start the Spot data receiving agent; an exception is thrown if it is already running
 * @param print print the progress of the received data
 * @param worker_num number of the worker task group threads used for the post-processing of the
 *                   received data
 * @param addr server address; empty means using the market data server address in the hikyuu config
 *             file
 * @ingroup Agent
 */
void HKU_API startSpotAgent(bool print = true, size_t worker_num = 1,
                            const string& addr = string());

/**
 * Terminate the Spot data receiving agent
 * @ingroup Agent
 */
void HKU_API stopSpotAgent();

HKU_API SpotAgent* getGlobalSpotAgent();

void HKU_API releaseGlobalSpotAgent();

inline bool spotAgentIsRunning() {
    return getGlobalSpotAgent()->isRunning();
}

inline bool spotAgentIsConnected() {
    return getGlobalSpotAgent()->isConnected();
}

}  // namespace hku