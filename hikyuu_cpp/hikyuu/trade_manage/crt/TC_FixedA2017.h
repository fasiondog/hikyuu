/*
 * TC_FixedA2015.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_FIXEDA2017_H_
#define TRADE_MANAGE_CRT_TC_FIXEDA2017_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share after August 1, 2015; it calculates
 * the cost of every buy or sell
 * The Shenzhen market also started to charge the transfer fee after January 1, 2017
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: the current commission ratio is 1.8 per mille (5 yuan minimum), and the stamp duty is
 *   1 per mille
 *        After 2015 the transfer fee of the Shanghai Stock Exchange is 0.00002 of the turnover
 * amount
 * </pre>
 *
 * @param commission commission ratio, 1.8 per mille by default, i.e. 0.0018
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @param stamptax stamp duty, 1 per mille by default, i.e. 0.001
 * @param transferfee transfer fee, 0.2 per mille by default, i.e. 0.00002
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_FixedA2017(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                                   price_t stamptax = 0.001, price_t transferfee = 0.00002);

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_FIXEDA2015_H_ */
