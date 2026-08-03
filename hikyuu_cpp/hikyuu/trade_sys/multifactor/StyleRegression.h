/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-27
 *      Author: woleigegg
 *
 *  Style factor neutralization regression helpers.
 */

#pragma once

#include "hikyuu/DataType.h"

namespace hku {

HKU_API PriceList calculate_style_residuals(const PriceList& y, const vector<PriceList>& x);

}  // namespace hku
