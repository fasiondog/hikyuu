/*
 * PRICELIST.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef PRICELIST_H_
#define PRICELIST_H_

#include "../Indicator.h"

#define VALUE PRICELIST

namespace hku {

/**
 * 包装PriceList成Indicator
 * @param data 源数据
 * @param discard 前端抛弃的数据点数，抛弃的值使用Null<price_t>()填充
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(const PriceList& data, int discard = 0);
Indicator HKU_API PRICELIST(const PriceList& data, const DatetimeList& ds, int discard = 0);
Indicator HKU_API PRICELIST();

/**
 * 包装数组成Indicator，用于计算其他指标
 * @param data price_t[]
 * @param total 数组大小
 * @ingroup Indicator
 */
// Indicator HKU_API PRICELIST(double* data, size_t total);
template <typename ValueT>
Indicator PRICELIST(ValueT* data, size_t total) {
    HKU_IF_RETURN(!data || total == 0, PRICELIST(PriceList()));
    PriceList tmp(total);
    std::copy(data, data + total, tmp.begin());
    return PRICELIST(tmp);
}

}  // namespace hku

#endif /* PRICELIST_H_ */
