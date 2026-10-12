/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Get the total share capital (unit: 10 thousand shares), taken from the ex-rights/ex-dividend
 * records of the stock and carried over between the records
 *
 * @note Needs a KData context to work; the bars before the first valid record are null
 *
 * @param kdata the associated K-line data
 * @ingroup Indicator
 */
Indicator HKU_API ZONGGUBEN();
Indicator HKU_API ZONGGUBEN(const KData&);

}  // namespace hku
