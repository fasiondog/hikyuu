/*
 * SP_LogNormal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * Log-normal distribution random price slippage algorithm: the sampled value is folded to a
 * non-negative amplitude and applied in the unfavorable direction, so the buy price is the plan
 * price plus the amplitude and the sell price is the plan price minus the amplitude.
 * @param mean the mean parameter of the log-normal distribution
 * @param stddev the standard deviation parameter of the log-normal distribution
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_LogNormal(double mean = 0.0, double stddev = 0.05);

} /* namespace hku */