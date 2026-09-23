/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Get the data of the given range [start, end) in a PriceList
 * @param data source data
 * @param start the start range, it can be negative
 * @param end the end range (excluded), it can be negative
 * @ingroup Indicator
 */
Indicator HKU_API SLICE(const PriceList& data, int64_t start, int64_t end);

/**
 * Get the data of the given range in an indicator
 * @param start the start range, it can be negative
 * @param end the end range (excluded), it can be negative
 * @param result_index the given result set in the source data, less than 0 means all
 * @ingroup Indicator
 */
Indicator HKU_API SLICE(int64_t start, int64_t end, int result_index = -1);

/**
 * Get the data of the given range in an indicator
 * @param ind source data
 * @param start the start range, it can be negative
 * @param end the end range (excluded), it can be negative
 * @param result_index the given result set in the source data, less than 0 means all
 * @ingroup Indicator
 */
inline Indicator SLICE(const Indicator& ind, int64_t start, int64_t end, int result_index = -1) {
    return SLICE(start, end, result_index)(ind);
}

}  // namespace hku
