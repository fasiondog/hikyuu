/*
 * SYS_Simple.cpp
 *
 *  Created on: 2013-5-7
 *      Author: fasiondog
 */

#include "../crt/SYS_Simple.h"

namespace hku {

SystemPtr HKU_API SYS_Simple(const TradeManagerPtr& tm, const MoneyManagerPtr& mm,
                             const EnvironmentPtr& ev, const ConditionPtr& cn, const SignalPtr& sg,
                             const StoplossPtr& st, const StoplossPtr& tp, const ProfitGoalPtr& pg,
                             const SlippagePtr& sp) {
    return make_shared<System>(tm, mm, ev, cn, sg, st, tp, pg, sp, "SYS_Simple");
}

} /* namespace hku */
