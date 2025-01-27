/*
 * HSL.h
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_HSL_H_
#define INDICATOR_CRT_HSL_H_

#include "KDATA.h"
#include "LIUTONGPAN.h"
#include "HSL.h"

namespace hku {

/**
 * 获取换手率, 乘以 100 才是百分比，等于 VOL(k) / CAPITAL(k) * 0.01
 * @param k 关联的K线数据
 * @ingroup Indicator
 */
Indicator HSL(const KData& k);
Indicator HSL();

}  // namespace hku
#endif /* INDICATOR_CRT_HSL_H_ */
