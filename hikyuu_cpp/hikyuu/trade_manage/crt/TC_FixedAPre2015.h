/*
 * TC_FixedAPre2015.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_FIXEDAPRE2015_H_
#define TRADE_MANAGE_CRT_TC_FIXEDAPRE2015_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share before August 1, 2015
 * @details
 * <pre>
 * The transfer fee of the Shanghai Stock Exchange is charged by traded quantity (1 yuan per 1000
 * shares, minimum 1 yuan); the Shenzhen market does not charge the transfer fee before 2017.
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
TradeCostPtr HKU_API TC_FixedAPre2015(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                                      price_t stamptax = 0.001, price_t transferfee = 0.001,
                                      price_t lowestTransferfee = 1.0);

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_FIXEDAPRE2015_H_ */
