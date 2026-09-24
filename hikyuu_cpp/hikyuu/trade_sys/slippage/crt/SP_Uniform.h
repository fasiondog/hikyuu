/*
 * SL_FixedValue.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * Uniform distribution random price slippage algorithm: the buy and sell operations are a random
 * offset of the price with a uniform distribution within the range [min_value, max_value]
 * @param min_value the lower limit of the offset price
 * @param max_value the upper limit of the offset price
 * @return
 */
SlippagePtr HKU_API SP_Uniform(double min_value = -0.05, double max_value = 0.05);

} /* namespace hku */
