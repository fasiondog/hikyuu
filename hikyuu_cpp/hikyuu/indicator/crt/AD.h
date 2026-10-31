/*
 * AD.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_AD_H_
#define INDICATOR_CRT_AD_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Accumulation / distribution line, accumulated from the turnover amount, so the result is in the
 * same unit as the amount: 10 thousand yuan
 * @param k the associated KData
 * @note Bars with non-finite (High - Low) are skipped; the accumulator never gets poisoned by
 *       a single bad bar
 * @ingroup Indicator
 */
Indicator HKU_API AD(const KData& k);
Indicator HKU_API AD();

inline void registerPart_AD() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "AD";
    meta.params.push_back(ParamMeta{.name = "k", .type = "KData", .required = true});
    registerIndicatorPart(
      "core.ind.ad", meta,
      [](const Parameter& sc, const PartParamMap&) -> Part { return AD(sc.get<KData>("k")); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_AD_H_ */
