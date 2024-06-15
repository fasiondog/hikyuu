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
 * 横向统计（返回板块股个数）
 * @param blk 待统计的板块
 * @param query 统计范围
 * @return Indicator
 */
Indicator HKU_API BLOCKSETNUM(const Block& blk, const KQuery& query);
Indicator HKU_API BLOCKSETNUM(const Block& blk);

}  // namespace hku