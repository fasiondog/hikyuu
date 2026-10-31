/*
 * PartRegistration.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "hikyuu/part/PartRegistration.h"

namespace hku {

void registerAllBuiltinParts() {
    registerIndicatorParts();
    registerSignalParts();
    registerConditionParts();
    registerEnvironmentParts();
    registerMoneyManagerParts();
    registerStoplossParts();
    registerProfitGoalParts();
    registerSlippageParts();
    registerPortfolioParts();
    registerSelectorParts();
    registerAllocateFundsParts();
    registerMultiFactorParts();
    registerSystemParts();
    registerTradeManageParts();
}

}  // namespace hku
