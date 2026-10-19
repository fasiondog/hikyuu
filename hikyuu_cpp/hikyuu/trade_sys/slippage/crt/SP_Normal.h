/*
 * SP_Normal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#pragma once

#include <optional>
#include "../SlippageBase.h"

namespace hku {

/**
 * Normal distribution random price slippage algorithm.
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 * @param adverse_only if true (default), fold sampled value to non-negative amplitude and apply
 *        in the unfavorable direction; if false, apply the raw signed value (zero-mean execution
 *        noise, not necessarily symmetric)
 * @param short_mean mean of the short-side distribution; left unset it takes mean
 * @param short_stddev standard deviation of the short-side distribution; left unset it takes stddev
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_Normal(double mean = 0.0, double stddev = 0.05, bool adverse_only = true,
                              std::optional<double> short_mean = std::nullopt,
                              std::optional<double> short_stddev = std::nullopt);

} /* namespace hku */