/*
 * SMA.h
 *
 *  Created on: 2017年4月19日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_WEAVE_H_
#define INDICATOR_CRT_WEAVE_H_

#include "../Indicator.h"

namespace hku {

/**
 * 将ind的结果组合在一起放在一个Indicator中。如ind = WEAVE(ind1); ind=ind(ind2)。
 * 则此时ind包含两个结果，result1为ind1的数据,result2为ind2的数据
 * @param data 待计算的数据
 * @ingroup Indicator
 */
Indicator HKU_API WEAVE(const Indicator& ind);

Indicator HKU_API WEAVE();

} /* namespace */



#endif /* INDICATOR_CRT_WEAVE_H_ */
