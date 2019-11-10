/*
 * VIGOR.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef VIGOR_H_
#define VIGOR_H_

#include "../Indicator.h"

namespace hku {

/**
 * 亚历山大 艾尔德力度指数
 * @details
 * <pre>
 * 参见《走进我的交易室》（2007年 地震出版社） (Alexander Elder) P131
 * 计算公式：（收盘价今－收盘价昨）＊成交量今
 * 一般可以再使用EMA或MA进行平滑
 * </pre>
 * @param kdata 待计算的K线数据
 * @param n EMA平滑窗口，必须大于等于1
 * @ingroup Indicator
 */
Indicator HKU_API VIGOR(const KData& kdata, int n = 2);

Indicator HKU_API VIGOR(int n = 2);

}  // namespace hku

#endif /* VIGOR_H_ */
