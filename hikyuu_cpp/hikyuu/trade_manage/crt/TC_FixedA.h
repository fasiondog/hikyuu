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
 * Trade cost algorithm for the Shanghai and Shenzhen A-share; it calculates the cost of every buy
 * or sell
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: both the commission and the transfer fee have a minimum value; the current commission
 *   ratio is 1.8 per mille (5 yuan minimum), and the stamp duty is 1 per mille
 *         The transfer fee of the Shanghai Stock Exchange is 1 per mille of the traded quantity,
 * and it is counted as one yuan when it is less than 1 yuan
 * </pre>
 *
 * @param commission commission ratio, 1.8 per mille by default, i.e. 0.0018
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @param stamptax stamp duty, 1 per mille by default, i.e. 0.001
 * @param transferfee transfer fee, 1 per mille per share by default, i.e. 0.001
 * @param lowestTransferfee minimum transfer fee, 1 yuan by default
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_FixedA(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                               price_t stamptax = 0.001, price_t transferfee = 0.001,
                               price_t lowestTransferfee = 1.0);

}  // namespace hku

#endif /* CRTFIXEDATC_H_ */
