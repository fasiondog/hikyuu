/*
 * SL_FixedValue.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#pragma once

#include <optional>
#include "../SlippageBase.h"

namespace hku {

/**
 * Uniform distribution random price slippage algorithm.
 * @param min_value the lower limit of the offset range
 * @param max_value the upper limit of the offset range
 * @param adverse_only if true (default), fold sampled value to non-negative amplitude and apply
 *        in the unfavorable direction; if false, apply the raw signed value (zero-mean execution
 *        noise, not necessarily symmetric)
 * @param short_min_value lower limit of the short-side offset range; left unset it takes min_value
 * @param short_max_value upper limit of the short-side offset range; left unset it takes max_value
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_Uniform(double min_value = -0.05, double max_value = 0.05,
                               bool adverse_only = true,
                               std::optional<double> short_min_value = std::nullopt,
                               std::optional<double> short_max_value = std::nullopt);

} /* namespace hku */
