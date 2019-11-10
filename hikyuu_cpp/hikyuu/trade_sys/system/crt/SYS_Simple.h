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
 * 生成简单系统实例
 * @details
 * <pre>
 * 继承的系统参数：
 * max_delay_count [int | 3]: 最大连续延迟交易请求的限制次数，如最高价=最低价时
 * delay_use_plan_number [bool | true]:
 * 延迟操作时使用上一时刻计算的数量，因为实际人工操作时，可能无法实时计算买入数量
 * support_borrow_cash [bool | false]：在现金不足时，是否支持借入现金，融资
 * support_borrow_stock [bool | fals]): 在没有持仓时，是否支持借入证券，融券
 *
 * 本系统参数：
 * ev_dealy [bool | true]: 系统环境失效时，是否延迟在下一时刻开盘执行
 * cn_delay [bool | true]: 系统条件失效时，是否延迟在下一时刻开盘执行
 * sg_delay [bool | true]: 发出信号指示时，是否延迟在下一时刻开盘执行
 * sl_delay [bool | true]: 发出止损指令时，是否延迟在下一时刻开盘执行
 * tp_delay [bool | true]: 发出止赢指令时，是否延迟在下一时刻开盘执行
 * pg_delay [bool | true]: 发出盈利目标达成信号时，是否延迟在下一时刻开盘执行
 * tp_ascend [bool | true]: 止赢是否单调递增
 * tp_delay_n [int | 3]: 止赢延迟判定生效时间，给市场足够的时间向预定方向移动
 * ignore_sell_sg [bool | false ]: ignore_sell_sg 忽略卖出信号，只使用止损/止赢等其他方式卖出
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
