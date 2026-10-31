/*
 * DMA.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2015-5-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DMA_H_
#define INDICATOR_CRT_DMA_H_

#include "REF.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Dynamic moving average

 * @details
 * <pre>
 * Usage: DMA(X,A) gives the dynamic moving average of X.

 * Algorithm: if Y=DMA(X,A) then Y=A*X+(1-A)*Y', where Y' is the Y value of the previous period.

 * For example: DMA(CLOSE,VOL/CAPITAL) gives the average price with the turnover rate as the
 * smoothing factor

 * NaN/Inf handling: if x[i] or a[i] is non-finite (NaN or ±Inf), y[i] holds y[i-1] instead of
 poisoning the tail.

 * </pre>
 * @param x the data to be calculated

 * @param a dynamic coefficient

 * @param fill_null fill the missing data with nan when the dates are aligned

 * @ingroup Indicator
 */
Indicator HKU_API DMA(const Indicator& x, const Indicator& a, bool fill_null = true);

inline void registerPart_DMA() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "DMA";
    meta.params.push_back(
      ParamMeta{.name = "x", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "a", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "fill_null", .type = "bool", .default_value = "true"});
    registerIndicatorPart(
      "core.ind.dma", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return DMA(std::get<Indicator>(pp.at("x").at(0)), std::get<Indicator>(pp.at("a").at(0)),
                     sc.get<bool>("fill_null"));
      });
}

}  // namespace hku

#endif /* INDICATOR_CRT_DMA_H_ */
