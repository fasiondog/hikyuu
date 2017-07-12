/*
 * AND.h
 *
 *  Created on: 2017年7月13日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_IND_AND_H_
#define INDICATOR_CRT_IND_AND_H_

#include "../Indicator.h"

namespace hku {

/**
 * 对两个Indicator执行与操作
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果ind1相同位置的值和ind2相同位置的值都大于0则为1.0，否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator IND_AND(const Indicator&, const Indicator&);
HKU_API Indicator IND_AND(const Indicator&, price_t);
HKU_API Indicator IND_AND(price_t, const Indicator&);

/**
 * 对两个Indicator执行或操作
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果ind1相同位置的值和ind2相同位置的值其中一个大于0则为1.0，否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator IND_OR(const Indicator&, const Indicator&);
HKU_API Indicator IND_OR(const Indicator&, price_t);
HKU_API Indicator IND_OR(price_t, const Indicator&);

} /* namespace */

#endif /* INDICATOR_CRT_IND_AND_H_ */
