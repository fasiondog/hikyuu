/*
 * FixedPercent_SL.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDPERCENT_SL_H_
#define FIXEDPERCENT_SL_H_

#include "../StoplossBase.h"

namespace hku {

/**
 * Fixed percentage stop-loss strategy, i.e. the stop-loss is triggered when the price is lower than
 * the buy price by a certain percentage
 * @param p percentage (0,1]
 * @ingroup Stoploss
 */
StoplossPtr HKU_API ST_FixedPercent(double p = 0.03);

}  // namespace hku

#endif /* FIXEDPERCENT_SL_H_ */
