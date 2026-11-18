/*
 * TC_MarginInterest.h
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_MARGIN_INTEREST_H_
#define TRADE_MANAGE_CRT_TC_MARGIN_INTEREST_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Wrap another cost model with the interest of the margin financing and the securities lending
 * @details The wrapped model answers the trading fees; the rates here are used in daily settlement
 * to prorate the outstanding debt over the calendar days since the last billing and add the result
 * to the debt. Both rates are 0.0 by default, so wrapping alone bills nothing.
 *
 * @param trade_cost the model answering the trading fees, a zero-cost model by default
 * @param borrow_cash_rate annualized rate of the borrowed cash, [0, 1), 0.0 by default
 * @param borrow_stock_rates "key:rate" list of annualized lending rates in [0, 1)
 *        (e.g. "12:0.10, SHF:0.12"); an all-digit key is a security type, otherwise a market code
 * @param default_borrow_stock_rate rate of a security matched by no key, [0, 1), 0.0 by default
 * @see MarginInterestTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_MarginInterest(const TradeCostPtr& trade_cost = TradeCostPtr(),
                                       double borrow_cash_rate = 0.0,
                                       const string& borrow_stock_rates = "",
                                       double default_borrow_stock_rate = 0.0);

void registerPart_TC_MarginInterest();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_MARGIN_INTEREST_H_ */
