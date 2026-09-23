/*
 * TC_FixedETF.h
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_FIXEDETF_H_
#define TRADE_MANAGE_CRT_TC_FIXEDETF_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the ETF; it calculates the cost of every buy or sell
 * @details
 * <pre>
 * The calculation rules are:
 *   Buy: commission (5 yuan minimum)
 *   Sell: commission (5 yuan minimum)
 *   Where: the commission ratio is 0.1 per ten thousand (0.0001) by default, and the minimum
 *   commission is 5 yuan
 * </pre>
 *
 * @param commission commission ratio, 0.1 per ten thousand by default, i.e. 0.0001
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @see FixedETFTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_FixedETF(price_t commission = 0.0001, price_t lowestCommission = 5.0);

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_FIXEDETF_H_ */