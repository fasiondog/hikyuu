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
 * 将ind1和ind2的结果组合在一起放在一个Indicator中。如ind = WEAVE(ind1, ind2)
 * 则此时ind包含多个结果，按ind1、ind2的顺序存放
 * @param data 待计算的数据
 * @ingroup Indicator
 */
Indicator HKU_API WEAVE(const Indicator& ind1, const Indicator& ind2);

} /* namespace */



#endif /* INDICATOR_CRT_WEAVE_H_ */
