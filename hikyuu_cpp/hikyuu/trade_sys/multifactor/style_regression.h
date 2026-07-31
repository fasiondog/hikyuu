/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Style factor neutralization regression helpers.
 */

#pragma once

#include "hikyuu/DataType.h"

namespace hku {

HKU_API PriceList calculate_style_residuals(const PriceList& y, const vector<PriceList>& x);

}  // namespace hku
