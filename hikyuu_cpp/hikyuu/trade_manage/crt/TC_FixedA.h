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
 * Trade cost algorithm for the Shanghai, Shenzhen and Beijing A-share at the current rates; it is
 * the rolling default and always reflects the latest regime
 * @details
 * <pre>
 *   1) transfer fee: since 2022-04-29, 0.01 per mille of the turnover amount, charged in both
 *      directions on all markets
 *   2) stamp duty: since 2023-08-28, 0.5 per mille, charged on sells only
 *   3) commission: charged in both directions, 5 yuan minimum
 * When the fees are cut again, update this model's defaults; the year-named models
 * (TC_FixedAPre2015 / 2015 / 2017) stay frozen for historical backtests.
 * </pre>
 *
 * @param commission commission ratio, 1.8 per mille by default, i.e. 0.0018
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @param stamptax stamp duty, 0.5 per mille by default, i.e. 0.0005
 * @param transferfee transfer fee, 0.01 per mille of turnover by default, i.e. 0.00001
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_FixedA(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                               price_t stamptax = 0.0005, price_t transferfee = 0.00001);

}  // namespace hku

#endif /* CRTFIXEDATC_H_ */
