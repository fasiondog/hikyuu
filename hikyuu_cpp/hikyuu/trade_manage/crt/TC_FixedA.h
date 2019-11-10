/*
 * TC_FixedA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef CRTFIXEDATC_H_
#define CRTFIXEDATC_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * 沪深A股交易成本算法,计算每次买入或卖出的成本
 * @details
 * <pre>
 * 计算规则为：
 *   1）上证交易所
 *      买入：佣金＋过户费
 *      卖出：佣金＋过户费＋印花税
 *   2）深证交易所：
 *      买入：佣金
 *      卖出：佣金＋印花税
 *   其中：佣金和过户费均有最低值，当前佣金比例为千分之1.8（最低5元），印花税为千分之一
 *         上证过户费为交易数量的千分之一，不足1元，按一元计
 * </pre>
 *
 * @param commission 佣金比例，默认千分之1.8，即0.0018
 * @param lowestCommission 最低佣金值，默认5元
 * @param stamptax 印花税，默认千分之一，即0.001
 * @param transferfee 过户费，默认每股千分之一，即0.001
 * @param lowestTransferfee 最低过户费，默认1元
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_FixedA(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                               price_t stamptax = 0.001, price_t transferfee = 0.001,
                               price_t lowestTransferfee = 1.0);

}  // namespace hku

#endif /* CRTFIXEDATC_H_ */
