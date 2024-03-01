/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 10年期中国国债收益率
 * @note 优先使用上下文中日期作为参考日期
 * @param default_val 如果指定日期早于已有的最早国债数据，此时使用该指定默认值
 * @return Indicator
 */
Indicator HKU_API ZHBOND10(double default_val = 4.0);
Indicator HKU_API ZHBOND10(const DatetimeList& dates, double default_val = 4.0);
Indicator HKU_API ZHBOND10(const KData& k, double default_val = 4.0);
inline Indicator ZHBOND10(const Indicator& ind, double default_val = 4.0) {
    return ZHBOND10(default_val)(ind);
}

}  // namespace hku