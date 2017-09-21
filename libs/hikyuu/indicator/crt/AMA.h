/*
 * AMA.h
 *
 *  Created on: 2013-4-8
 *      Author: fasiondog
 */

#ifndef AMA_H_
#define AMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
 * @param n 计算均值的周期窗口，必须为大于2的整数，默认为10天
 * @param fast_n 对应快速周期N，默认为2
 * @param slow_n 对应慢速EMA线的N值，默认为30，不过当超过60左右该指标会收敛不会有太大的影响
 * @ingroup Indicator 具有2个结果集，result(0)为AMA，result(1)为ER
 */
Indicator HKU_API AMA(int n = 10, int fast_n = 2, int slow_n = 30);

/**
 * 佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
 * @param indicator 待计算的数据
 * @param n 计算均值的周期窗口，必须为大于2的整数，默认为10天
 * @param fast_n 对应快速周期N，默认为2
 * @param slow_n 对应慢速EMA线的N值，默认为30，不过当超过60左右该指标会收敛不会有太大的影响
 * @ingroup Indicator
 */
Indicator HKU_API AMA(const Indicator& indicator, int n = 10,
        int fast_n = 2, int slow_n = 30);

} /* namespace */

#endif /* AMA_H_ */
