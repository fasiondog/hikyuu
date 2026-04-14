/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * @brief 返回品种名称是否匹配指定模式
 * @param pattern 匹配模式，支持通配符*和?
 * @return Indicator
 */
Indicator HKU_API NAMELIKE(const string& pattern);

/**
 * @brief 返回品种名称是否匹配指定模式
 * @param kdata K线数据
 * @param pattern 匹配模式，支持通配符*和?
 * @return Indicator
 */
Indicator HKU_API NAMELIKE(const KData& kdata, const string& pattern);

}  // namespace hku
