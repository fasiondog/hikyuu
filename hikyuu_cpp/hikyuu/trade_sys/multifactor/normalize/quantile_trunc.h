/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"

namespace hku {

PriceList quantile_trunc(const PriceList &src, double quantile_min, double quantile_max);

}