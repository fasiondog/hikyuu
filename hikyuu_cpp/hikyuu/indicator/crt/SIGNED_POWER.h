/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SIGNED_POWER_H_
#define INDICATOR_CRT_SIGNED_POWER_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Power
 * @details
 * <pre>
 * SIGNED_POWER(A,B) returns A to the power of B, but keeps the original sign
 * For example: SIGNED_POWER(CLOSE,3) gives the cube of the close price and keeps the original sign
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API SIGNED_POWER(int n);
Indicator HKU_API SIGNED_POWER(const IndParam& n);

inline Indicator SIGNED_POWER(const Indicator& ind, int n) {
    return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const IndParam& n) {
    return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const Indicator& n) {
    return SIGNED_POWER(IndParam(n))(ind);
}

inline Indicator SIGNED_POWER(Indicator::value_t val, int n) {
    return SIGNED_POWER(CVAL(val), n);
}

inline void registerPart_SIGNED_POWER() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "SIGNED_POWER";
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .required = true});
    registerIndicatorPart("core.ind.signed_power", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return SIGNED_POWER(static_cast<int>(sc.get<int64_t>("n")));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_SIGNED_POWER_H_ */
