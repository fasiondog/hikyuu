/*
 * VIGOR.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_VIGOR_H_
#define INDICATOR_CRT_VIGOR_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Alexander Elder's force index

 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) (Alexander Elder) P131

 * Calculation formula: (today's close price - yesterday's close price) * today's volume

 * EMA or MA can generally be used for the smoothing afterwards

 * </pre>
 * @param kdata the K-line data to be calculated

 * @param n EMA smoothing window, it must be greater than or equal to 1

 * @ingroup Indicator
 */
Indicator HKU_API VIGOR(const KData& kdata, int n = 2);

Indicator HKU_API VIGOR(int n = 2);

inline void registerPart_VIGOR() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "VIGOR";
    meta.params.push_back(ParamMeta{.name = "kdata", .type = "KData", .required = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "2"});
    registerIndicatorPart(
      "core.ind.vigor", meta, [](const Parameter& sc, const PartParamMap&) -> Part {
          return VIGOR(sc.get<KData>("kdata"), static_cast<int>(sc.get<int64_t>("n")));
      });
}

}  // namespace hku

#endif /* INDICATOR_CRT_VIGOR_H_ */
