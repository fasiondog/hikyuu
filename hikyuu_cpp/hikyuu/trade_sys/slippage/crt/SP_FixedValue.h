/*
 * SL_FixedValue.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_
#define TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_

#include <optional>
#include "../SlippageBase.h"

namespace hku {

/**
 * Fixed price slippage algorithm
 * @details Actual buy price = planned buy price + offset price, actual sell price = planned sell
 *          price - offset price
 * @param value the fixed offset price
 * @param short_value the fixed offset price of the short side (opening a short and covering it),
 *        must be >= 0; left unset it takes the same value as value
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_FixedValue(double value = 0.01,
                                  std::optional<double> short_value = std::nullopt);

} /* namespace hku */

#endif /* TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDVALUE_H_ */
