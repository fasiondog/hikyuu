/*
 * AD.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_AD_H_
#define INDICATOR_CRT_AD_H_

#include "../Indicator.h"

namespace hku {

/**
 * 累积/派发线
 * @param k 关联的 KData 数据
 * @ingroup Indicator
 */
Indicator HKU_API AD(const KData& k);
Indicator HKU_API AD();

}  // namespace hku

#endif /* INDICATOR_CRT_AD_H_ */
