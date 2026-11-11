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
 * @details The wrapped model keeps answering the trading fees; the rates answered here are used by
 * the account in its daily settlement, prorating the outstanding debt over the calendar days gone
 * by since the book was last billed and adding the result to the debt itself. Both rates are 0.0
 * by default, i.e. wrapping a model alone bills nothing.
 *
 * @param trade_cost the model answering the trading fees, a zero-cost model by default
 * @param borrow_cash_rate annualized rate of the borrowed cash, [0, 1), 0.0 by default
 * @param borrow_stock_rates "key:rate" list of the annualized securities lending rates, each rate
 *        in [0, 1), e.g. "12:0.10, SHF:0.12"; an all-digit key is a security type, any other key is
 *        a market code
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
