/*
 * COST.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-19
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_COST_H_
#define INDICATOR_CRT_COST_H_

#include "KDATA.h"

namespace hku {

/**
 * 成本分布
 * @details
 * <pre>
 * 用法：COST(k, X) 表示X%获利盘的价格是多少
 * 例如：COST(k, 10),表示10%获利盘的价格是多少，即有10%的持仓量在该价格以下，
 *       其余90%在该价格以上，为套牢盘 该函数仅对日线分析周期有效
 * </pre>
 * @param k 关联的K线数据
 * @param x X%获利盘
 * @ingroup Indicator
 */
Indicator HKU_API COST(const KData& k, double x = 10.0);
Indicator HKU_API COST(double x = 10.0);

}  // namespace hku
#endif /* INDICATOR_CRT_COST_H_ */
