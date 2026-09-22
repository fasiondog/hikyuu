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
 * Wrap a PriceList into an Indicator
 * @param data source data
 * @param discard the number of the data points discarded at the front; the discarded values are
 *                 filled with Null<price_t>()
 * @ingroup Indicator
 */
Indicator HKU_API PRICELIST(const PriceList& data, int discard = 0);
Indicator HKU_API PRICELIST(PriceList&& data, int discard = 0);
Indicator HKU_API PRICELIST(const PriceList& data, const DatetimeList& ds, int discard = 0);
Indicator HKU_API PRICELIST(PriceList&& data, const DatetimeList&& ds, int discard = 0);
Indicator HKU_API PRICELIST(size_t size, double value, int discard = 0);
Indicator HKU_API PRICELIST(const DatetimeList& dates, double value, int discard = 0);
Indicator HKU_API PRICELIST(DatetimeList&& dates, double value, int discard = 0);
Indicator HKU_API PRICELIST();

/**
 * Wrap an array into an Indicator, used to calculate the other indicators
 * @param data price_t[]
 * @param total array size
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
