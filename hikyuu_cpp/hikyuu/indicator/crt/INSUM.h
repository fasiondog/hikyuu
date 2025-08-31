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
 * 返回板块各成分该指标相应输出按计算类型得到的计算值.
 * @note 注意: INSUM使用模式4/5时相当于RANK功能, 但不适合在MF中使用, 在 MF 中使用时计算量为 N x N
 * 级别, 计算缓慢。如果希望在 MF 中使用，建议直接使用 RANK[VIP] 指标。
 * @param block 指定板块
 * @param query 指定范围
 * @param ind 指定指标
 * @param mode 计算类型:0-累加,1-平均数,2-最大值,3-最小值,4-降序排名(指标值最高的排名值为1)
 * 5-升序排名(指标值最低的排名值为1),
 * @param fill_null 日期对齐时缺失数据填充 nan 值。
 * @return Indicator
 */
Indicator HKU_API INSUM(const Block& block, const KQuery& query, const Indicator& ind, int mode,
                        bool fill_null = true);

Indicator HKU_API INSUM(const Block& block, const Indicator& ind, int mode, bool fill_null = true);

Indicator HKU_API INSUM(const Block& block, int mode, bool fill_null);

}  // namespace hku