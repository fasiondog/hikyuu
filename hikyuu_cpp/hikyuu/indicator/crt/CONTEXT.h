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
 * @param ind 待包装指标
 * @param fill_null 是否填充空值，默认为 false
 * @param use_self_ktype 使用自身独立上下文的K线类型，否则计算时将使用计算上下文中K线类型
 * @param use_self_recover_type 使用自身独立上下文的复权类型，否则计算时将使用计算上下文中的复权类型
 * @ingroup Indicator
 */
Indicator HKU_API CONTEXT(const Indicator& ind, bool fill_null = false, bool use_self_ktype = false,
                          bool use_self_recover_type = false);
Indicator HKU_API CONTEXT(bool fill_null = false, bool use_self_ktype = false,
                          bool use_self_recover_type = false);

/**
 * 获取指标上下文
 * @note Indicator::getContext()方法获取的是当前的上下文，但对于 CONTEXT
 * 独立上下文指标无法获取其指定的独立上下文，需用此方法获取
 * @param ind
 * @return KData
 */
KData HKU_API CONTEXT_K(const Indicator& ind);

}  // namespace hku