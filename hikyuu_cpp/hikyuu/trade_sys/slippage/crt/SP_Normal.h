/*
 * SP_Normal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * Normal distribution random price slippage algorithm: the buy and sell operations are a random
 * price offset based on the normal distribution
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_Normal(double mean = 0.0, double stddev = 0.05);

} /* namespace hku */