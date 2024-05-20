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

/**
 * 横向统计（返回板块股个数）
 * @param category 板块分类
 * @param name 板块名称
 * @param query 统计范围
 * @return Indicator
 */
Indicator HKU_API BLOCKSETNUM(const string& category, const string& name, const KQuery& query);

}  // namespace hku