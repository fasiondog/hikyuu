/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 返回板块各成分该指标相应输出按计算类型得到的计算值.计算类型:0-累加,1-平均数,2-最大值,3-最小值.
 * @param block 指定板块
 * @param query 指定范围
 * @param ind 指定指标
 * @param mode 计算类型:0-累加,1-平均数,2-最大值,3-最小值.
 * @return Indicator
 */
Indicator HKU_API INSUM(const Block& block, const KQuery& query, const Indicator& ind, int mode);

/**
 * 返回板块各成分该指标相应输出按计算类型得到的计算值.计算类型:0-累加,1-平均数,2-最大值,3-最小值.
 * @param block 指定板块
 * @param ind 指定指标
 * @param mode 计算类型:0-累加,1-平均数,2-最大值,3-最小值.
 * @return Indicator
 */
Indicator HKU_API INSUM(const Block& block, const Indicator& ind, int mode);

}  // namespace hku