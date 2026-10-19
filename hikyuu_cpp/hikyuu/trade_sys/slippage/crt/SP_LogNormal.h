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
 * Log-normal distribution random price slippage algorithm.
 * @param mean the mean parameter of the log-normal distribution
 * @param stddev the standard deviation parameter of the log-normal distribution
 * @param adverse_only if true (default), fold sampled value to non-negative amplitude and apply
 *        in the unfavorable direction; if false, apply the raw centered value (zero-mean
 *        execution noise, not necessarily symmetric)
 * @note with adverse_only=false, mean only scales the dispersion and produces no drift
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_LogNormal(double mean = 0.0, double stddev = 0.05, bool adverse_only = true);

} /* namespace hku */