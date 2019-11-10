/*
 * Saftyloss_ST.h
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#pragma once
#ifndef SAFTYLOSS_ST_H_
#define SAFTYLOSS_ST_H_

#include "../imp/IndicatorStoploss.h"

namespace hku {

/**
 * 亚历山大 艾尔德安全地带止损
 * @details
 * <pre>
 * 参见《走进我的交易室》（2007年 地震出版社） 亚历山大.艾尔德(Alexander Elder) P202
 * 计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，
 *         得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日
 *         最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的
 *         上移，在上述结果的基础上再取起N日（一般为3天）内的最高值
 * </pre>
 * @note：返回结果中前（回溯周期宽度＋去最高值的宽度）个点是无效的
 * @see SAFTYLOSS
 * @param n1 计算平均噪音的回溯时间窗口，默认为10天
 * @param n2 对初步止损线去n2日内的最高值，默认为3
 * @param p 噪音系数，默认为2
 * @ingroup Stoploss
 */
StoplossPtr HKU_API ST_Saftyloss(int n1 = 10, int n2 = 3, double p = 2.0);

}  // namespace hku

#endif /* SAFTYLOSS_ST_H_ */
