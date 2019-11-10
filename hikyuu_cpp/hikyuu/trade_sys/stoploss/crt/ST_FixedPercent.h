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
 * 固定百分比止损策略，即当价格低于买入价格的某一百分比时止损
 * @param p 百分比(0,1]
 * @ingroup Stoploss
 */
StoplossPtr HKU_API ST_FixedPercent(double p = 0.03);

}  // namespace hku

#endif /* FIXEDPERCENT_SL_H_ */
