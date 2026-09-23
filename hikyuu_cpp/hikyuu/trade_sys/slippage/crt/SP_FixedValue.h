/*
 * SL_FixedValue.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_
#define TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_

#include "../SlippageBase.h"

namespace hku {

/**
 * Fixed price slippage algorithm
 * @details Actual buy price = planned buy price + offset price, actual sell price = planned sell
 *          price - offset price
 * @param value
 * @return
 */
SlippagePtr HKU_API SP_FixedValue(double value = 0.01);

} /* namespace hku */

#endif /* TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_ */
