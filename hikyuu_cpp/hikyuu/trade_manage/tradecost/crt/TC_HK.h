/*
 * TC_HK.h
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_HK_H_
#define TRADE_MANAGE_CRT_TC_HK_H_

#include "../TradeCostBase.h"
#include "../imp/FixedHKTradeCost.h"

namespace hku {

/**
 * The Hong Kong stock trade cost algorithm; buy and sell are charged symmetrically: commission
 * (with a per-order minimum), stamp duty, the exchange trading fee plus levies, and an optional
 * fixed charge per order
 * @param commission commission ratio (double-sided, of the turnover)
 * @param lowestCommission lowest commission value (double-sided, per order)
 * @param stamptax stamp duty rate (double-sided, 0.1% since 2023-11-17), rounded up to a whole HK
 *        dollar per order
 * @param tradefee aggregate of the exchange trading fee and the SFC / AFIRD levies (double-sided:
 *        exchange 0.00565% + SFC 0.0027% + AFIRD 0.00015%, default 0.000085)
 * @param perTrade fixed charge per order (double-sided)
 * @param settlefee CCASS settlement fee ratio (double-sided, default 0 to disable)
 * @param settleFeeMin settlement fee floor (default 0 for no lower bound)
 * @param settleFeeMax settlement fee cap (default 0 for no upper bound)
 * @return TradeCostPtr
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_HK(price_t commission = 0.00025, price_t lowestCommission = 3.0,
                           price_t stamptax = 0.001, price_t tradefee = 0.000085,
                           price_t perTrade = 0.0, price_t settlefee = 0.0,
                           price_t settleFeeMin = 0.0, price_t settleFeeMax = 0.0);

void registerPart_TC_HK();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_HK_H_ */
