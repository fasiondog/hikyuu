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
 * Log-normal distribution random price slippage algorithm: the buy and sell operations are a random
 * price offset based on the log-normal distribution
 * @param mean the mean parameter of the log-normal distribution
 * @param stddev the standard deviation parameter of the log-normal distribution
 * @return the slippage object pointer
 */
SlippagePtr HKU_API SP_LogNormal(double mean = 0.0, double stddev = 0.05);

} /* namespace hku */