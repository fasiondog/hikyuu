/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * @brief 返回品种是否属于某板块
 * @param category 指定板块分类
 * @param name 板块名称
 * @return Indicator
 */
Indicator HKU_API INBLOCK(const string& category, const string& name);

/**
 * @brief 返回品种是否属于某板块
 * @param kdata K线数据
 * @param category 指定板块分类
 * @param name 板块名称
 * @return Indicator
 */
Indicator HKU_API INBLOCK(const KData& kdata, const string& category, const string& name);

}  // namespace hku