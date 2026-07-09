/*
 * TC_FixedETF.h
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_FIXEDETF_H_
#define TRADE_MANAGE_CRT_TC_FIXEDETF_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * ETF交易成本算法,计算每次买入或卖出的成本
 * @details
 * <pre>
 * 计算规则为：
 *   买入：佣金（最低5元）
 *   卖出：佣金（最低5元）
 *   其中：佣金比例默认万分之1（0.0001），最低佣金5元
 * </pre>
 *
 * @param commission 佣金比例，默认万分之1，即0.0001
 * @param lowestCommission 最低佣金值，默认5元
 * @see FixedETFTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_FixedETF(price_t commission = 0.0001, price_t lowestCommission = 5.0);

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_FIXEDETF_H_ */