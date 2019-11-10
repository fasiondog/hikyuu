/*
 * TC_FixedA2015.h
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_FIXEDA2017_H_
#define TRADE_MANAGE_CRT_TC_FIXEDA2017_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * 2015年8月1日后沪深A股交易成本算法,计算每次买入或卖出的成本
 * 2017年1月1日后深市也开始收取过户费
 * @details
 * <pre>
 * 计算规则为：
 *   1）上证交易所
 *      买入：佣金＋过户费
 *      卖出：佣金＋过户费＋印花税
 *   2）深证交易所：
 *      买入：佣金
 *      卖出：佣金＋印花税
 *   其中：当前佣金比例为千分之1.8（最低5元），印花税为千分之一
 *        2015年后上证过户费为成交金额的0.00002
 * </pre>
 *
 * @param commission 佣金比例，默认千分之1.8，即0.0018
 * @param lowestCommission 最低佣金值，默认5元
 * @param stamptax 印花税，默认千分之一，即0.001
 * @param transferfee 过户费，默认千分之0.2，即0.00002
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_FixedA2017(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                                   price_t stamptax = 0.001, price_t transferfee = 0.00002);

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_FIXEDA2015_H_ */
