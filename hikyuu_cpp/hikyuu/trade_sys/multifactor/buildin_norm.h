/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "NormalizeBase.h"

namespace hku {

/**
 * @ingroup MultiFactor
 * @{
 */

inline NormPtr NORM_NOTHING() {
    return NormPtr();
}

/** Min-max standardization */
NormPtr HKU_API NORM_MinMax();

/** Normal standardization */
NormPtr HKU_API NORM_Zscore(bool outExtreme = false, double nsigma = 3.0, bool recursive = false);

/** Quantile distribution standardization */
NormPtr HKU_API NORM_Quantile(double quantile_min = 0.01, double quantile_max = 0.99);

/** Quantile uniform distribution standardization */
NormPtr HKU_API NORM_Quantile_Uniform(double quantile_min = 0.01, double quantile_max = 0.99);

/* @} */
}  // namespace hku