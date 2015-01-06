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
 * 包装PriceList成Indicator，用于计算其他指标
 * @param data 待包装的PriceList
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(const PriceList& data);

/**
 * 包装PriceList成Indicator, 同时只获取指定范围[start, end)的数据
 * @param data 源数据
 * @param start 数据起始位置
 * @param end 数据结束位置
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(const PriceList& data, size_t start, size_t end);

/**
 * 截取某indicator指定范围[start, end)的数据，并返回独立的Indicator
 * @param data 源数据
 * @param result_num 源数据中指定的结果集
 * @param start 起始位置
 * @param end 结束位置
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(const Indicator& data, size_t result_num,
        size_t start, size_t end);

/**
 * 包装 price_t 数组成Indicator，用于计算其他指标
 * @param data price_t[]
 * @param total 数组大小
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(price_t *data, size_t total);


} /* namespace */


#endif /* PRICELIST_H_ */
