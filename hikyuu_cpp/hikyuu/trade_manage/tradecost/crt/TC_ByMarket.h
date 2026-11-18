/*
 * TC_ByMarket.h
 *
 *  Created on: 2026-11-12
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_BYMARKET_H_
#define TRADE_MANAGE_CRT_TC_BYMARKET_H_

#include "../TradeCostBase.h"
#include "../imp/ByMarketTradeCost.h"

namespace hku {

/**
 * Route costs by market / security type: charges nothing itself, forwarding buy / sell / securities
 * lending fees to the model registered for the security's market or type, or to the default when
 * unregistered
 * @param defaultCost fallback for unregistered markets; a null pointer uses the zero-cost model
 * @return TradeCostPtr; the caller may add more registrations via add
 * @note Match the market code first (e.g. "HK"), then the type (e.g. "type:12"); configure before
 *       the backtest starts
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_ByMarket(const TradeCostPtr& defaultCost = TradeCostPtr());

void registerPart_TC_ByMarket();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_BYMARKET_H_ */
