/*
 * crtSimpleSystem.h
 *
 *  Created on: 2013-3-22
 *      Author: fasiondog
 */

#pragma once
#ifndef CRTSIMPLESYSTEM_H_
#define CRTSIMPLESYSTEM_H_

#include "../System.h"

namespace hku {

/**
 * Create a simple system instance
 * @details
 * <pre>
 * Inherited system parameters:
 * max_delay_count [int | 3]: the maximum number of the consecutive delayed trade requests, such as
 * when the high price equals the low price
 * delay_use_plan_number [bool | true]:
 * A delayed operation uses the quantity calculated at the previous moment, because in the actual
 * manual operation the buy quantity may not be calculated in real time
 * support_borrow_cash [bool | false]: whether borrowing cash is supported when the cash is
 * insufficient, i.e. margin financing
 * support_borrow_stock [bool | fals]): whether borrowing securities is supported when there is no
 * position, i.e. securities lending
 *
 * Parameters of this system:
 * ev_dealy [bool | true]: whether to delay the execution to the open of the next moment when the
 * system environment becomes invalid
 * cn_delay [bool | true]: whether to delay the execution to the open of the next moment when the
 * system condition becomes invalid
 * sg_delay [bool | true]: whether to delay the execution to the open of the next moment when a
 * signal indication is issued
 * sl_delay [bool | true]: whether to delay the execution to the open of the next moment when a
 * stop-loss instruction is issued
 * tp_delay [bool | true]: whether to delay the execution to the open of the next moment when a
 * take-profit instruction is issued
 * pg_delay [bool | true]: whether to delay the execution to the open of the next moment when the
 * profit goal reaching signal is issued
 * tp_ascend [bool | true]: whether the take-profit increases monotonically
 * tp_delay_n [int | 3]: the effective time of the take-profit delay judgment, it gives the market
 * enough time to move in the expected direction
 * ignore_sell_sg [bool | false ]: ignore_sell_sg ignores the sell signal and sells with the other
 * ways such as the stop-loss / take-profit only
 * </pre>
 * @see SystemBase
 * @ingroup System
 */
SystemPtr HKU_API SYS_Simple(
  const TradeManagerPtr& tm = TradeManagerPtr(), const MoneyManagerPtr& mm = MoneyManagerPtr(),
  const EnvironmentPtr& ev = EnvironmentPtr(), const ConditionPtr& cn = ConditionPtr(),
  const SignalPtr& sg = SignalPtr(), const StoplossPtr& sl = StoplossPtr(),
  const StoplossPtr& tp = StoplossPtr(), const ProfitGoalPtr& pg = ProfitGoalPtr(),
  const SlippagePtr& sp = SlippagePtr());

}  // namespace hku

#endif /* CRTSIMPLESYSTEM_H_ */
