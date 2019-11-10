/*
 * MM_FixedPercent.h
 *
 *  Created on: 2015年4月5日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDPERCENT_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDPERCENT_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * 百分比风险模型
 * @details 参见：《通往财务自由之路》2008年6月 机械工业出版社 范 K.撒普（Van K. Tharp.）P312
 *          公式：P（头寸规模）＝ C（总风险）/ R（每股的风险） ［这里C现金为总风险］
 * @param p 每笔交易总风险占总资产的百分比，如0.02表示总资产的2%
 * @ingroup MoneyManager
 */
MoneyManagerPtr HKU_API MM_FixedPercent(double p);

}  // namespace hku

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDPERCENT_H_ */
