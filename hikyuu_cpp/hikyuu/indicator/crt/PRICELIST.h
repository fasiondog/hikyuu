/*
 * PRICELIST.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_PRICELIST_H_
#define INDICATOR_CRT_PRICELIST_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

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

inline void registerPart_PRICELIST() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "PRICELIST";
    meta.params.push_back(ParamMeta{.name = "data", .type = "PriceList", .required = true});
    meta.params.push_back(ParamMeta{.name = "discard", .type = "int", .default_value = "0"});
    registerIndicatorPart(
      "core.ind.pricelist", meta, [](const Parameter& sc, const PartParamMap&) -> Part {
          return PRICELIST(sc.get<PriceList>("data"), static_cast<int>(sc.get<int64_t>("discard")));
      });
}

}  // namespace hku

#endif /* INDICATOR_CRT_PRICELIST_H_ */
