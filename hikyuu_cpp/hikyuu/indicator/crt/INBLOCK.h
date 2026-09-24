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
 * @brief Return whether the security belongs to a certain block

 * @param category the given block category

 * @param name block name

 * @return Indicator
 */
Indicator HKU_API INBLOCK(const string& category, const string& name);

/**
 * @brief Return whether the security belongs to a certain block

 * @param kdata K-line data

 * @param category the given block category

 * @param name block name

 * @return Indicator
 */
Indicator HKU_API INBLOCK(const KData& kdata, const string& category, const string& name);

}  // namespace hku