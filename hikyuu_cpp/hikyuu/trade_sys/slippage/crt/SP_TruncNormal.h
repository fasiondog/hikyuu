/*
 * SP_TruncNormal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * Truncated normal distribution random price slippage algorithm.
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 * @param min_value the truncation minimum
 * @param max_value the truncation maximum
 * @param adverse_only if true (default), fold sampled value to non-negative amplitude and apply
 *        in the unfavorable direction; if false, apply the raw signed value (zero-mean execution
 *        noise, not necessarily symmetric)
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_TruncNormal(double mean = 0.0, double stddev = 0.05, double min_value = -0.1,
                                   double max_value = 0.1, bool adverse_only = true);

} /* namespace hku */