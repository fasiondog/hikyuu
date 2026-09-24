/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Perform a ZScore standardization (3sigma) on the data for the extreme value handling
 * @note It is not a window rolling one; for a window rolling standardization, just use
 *       (x - MA(x, n)) / STDEV(x, n)
 * @param outExtreme indicates the removal of the extreme values (i.e. truncating the extreme
 *                    values, the ones exceeding the limit are replaced with the limit value)
 * @param nsigma the nsigma multiple of sigma used when removing the extreme values
 * @param recursive whether to remove the extreme values recursively
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API ZSCORE(bool outExtreme = false, double nsigma = 3.0, bool recursive = false);

inline Indicator ZSCORE(const Indicator& data, bool outExtreme = false, double nsigma = 3.0,
                        bool recursive = false) {
    return ZSCORE(outExtreme, nsigma, recursive)(data);
}

}  // namespace hku