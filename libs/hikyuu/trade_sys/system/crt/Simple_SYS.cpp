/*
 * Simple_SYS.cpp
 *
 *  Created on: 2013-5-7
 *      Author: fasiondog
 */

#include "Simple_SYS.h"

namespace hku {

SystemPtr HKU_API Simple_SYS(
        const TradeManagerPtr& tm,
        const MoneyManagerPtr& mm,
        const EnvironmentPtr& ev,
        const ConditionPtr& cn,
        const SignalPtr& sg,
        const StoplossPtr& sl,
        const StoplossPtr& tp,
        const ProfitGoalPtr& pg,
        const SlippagePtr& sp) {
    return SystemPtr(new System(tm, mm, ev, cn, sg, sl, tp, pg, sp,
            "Simple_SYS"));
}

} /* namespace hku */


