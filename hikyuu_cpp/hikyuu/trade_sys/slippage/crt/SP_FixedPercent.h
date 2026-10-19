/*
 * SL_FixedPercent.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDPERCENT_H_
#define TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDPERCENT_H_

#include <optional>
#include "../SlippageBase.h"

namespace hku {

/**
 * Fixed percentage slippage algorithm
 * @details Actual buy price = planned buy price * (1 + p), actual sell price = planned sell price *
 *          (1 - p)
 * @param p the fixed offset percentage
 * @param short_p the fixed offset percentage of the short side (opening a short and covering it),
 *        in [0, 1); left unset it takes the same value as p
 * @return SPPtr
 */
SlippagePtr HKU_API SP_FixedPercent(double p = 0.001, std::optional<double> short_p = std::nullopt);

} /* namespace hku */

#endif /* TRADE_SYS_SLIPPAGE_CRT_SP_FIXEDPERCENT_H_ */
