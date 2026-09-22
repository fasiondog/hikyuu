/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Cross-sectional statistics (it returns the number of the stocks in the block)
 * @param blk the block to be counted
 * @param query the statistics range
 * @return Indicator
 */
Indicator HKU_API BLOCKSETNUM(const Block& blk, const KQuery& query);
Indicator HKU_API BLOCKSETNUM(const Block& blk);

}  // namespace hku