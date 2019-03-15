/*
 * PRICELIST.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#ifndef PRICELIST_H_
#define PRICELIST_H_

#include "../Indicator.h"

namespace hku {

/**
 * 包装PriceList成Indicator
 * @param data 源数据
 * @param discard 前端抛弃的数据点数，抛弃的值使用Null<price_t>()填充
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(const PriceList&, int discard = 0);

/**
 * 将某指标转化为PRICELIST
 * @param ind 源数据
 * @param result_index 源数据中指定的结果集
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(const Indicator& ind, int result_index = 0);

/**
 * 将某指标转化为PRICELIST
 * @param result_index 源数据中指定的结果集
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(int result_index = 0);

/**
 * 包装 price_t 数组成Indicator，用于计算其他指标
 * @param data price_t[]
 * @param total 数组大小
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(price_t *data, size_t total);


} /* namespace */


#endif /* PRICELIST_H_ */
