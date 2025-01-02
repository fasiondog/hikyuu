/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 独立上下文指标
 * @ingroup Indicator
 */
Indicator HKU_API CONTEXT(const Indicator& ind, bool fill_null = true);
Indicator HKU_API CONTEXT(bool fill_null = true);

/**
 * 获取指标上下文
 * @note Indicator::getContext()方法获取的是当前的上下文，但对于 CONTEXT
 * 独立上下文指标无法获取其指定的独立上下文，需用此方法获取
 * @param ind
 * @return KData
 */
KData HKU_API CONTEXT_K(const Indicator& ind);

}  // namespace hku