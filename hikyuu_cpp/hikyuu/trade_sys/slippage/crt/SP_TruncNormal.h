/*
 * SP_TruncNormal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

#include <optional>
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
 * @param short_mean mean of the short-side distribution; left unset it takes mean
 * @param short_stddev standard deviation of the short-side distribution; left unset it takes stddev
 * @param short_min_value lower truncation bound of the short side; left unset it takes min_value
 * @param short_max_value upper truncation bound of the short side; left unset it takes max_value
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_TruncNormal(double mean = 0.0, double stddev = 0.05, double min_value = -0.1,
                                   double max_value = 0.1, bool adverse_only = true,
                                   std::optional<double> short_mean = std::nullopt,
                                   std::optional<double> short_stddev = std::nullopt,
                                   std::optional<double> short_min_value = std::nullopt,
                                   std::optional<double> short_max_value = std::nullopt);

} /* namespace hku */