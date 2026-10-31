/*
 * LONGCROSS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LONGCROSS_H_
#define INDICATOR_CRT_LONGCROSS_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"
#include "EVERY.h"
#include "REF.h"

namespace hku {

/**
 * Two lines cross after maintaining for a certain number of periods
 * @details
 * <pre>
 * LONGCROSS(A,B,N) means A is less than B within N periods,
 * and it returns 1 when A crosses B upward from below in the current period, otherwise 0.
 * </pre>
 * @ingroup Indicator
 */

inline Indicator LONGCROSS(const Indicator& x, const Indicator& y, int n = 3) {
    Indicator result = EVERY((REF(x, 1) < REF(y, 1)), n) & (x > y);
    result.name("LONGCROSS");
    return result;
}

inline Indicator LONGCROSS(const Indicator& x, const Indicator& y, const Indicator& n) {
    Indicator result = EVERY((REF(x, 1) < REF(y, 1)), n) & (x > y);
    result.name("LONGCROSS");
    return result;
}

inline Indicator LONGCROSS(const Indicator& x, Indicator::value_t y, int n = 3) {
    return LONGCROSS(x, CVAL(x, y), n);
}

inline Indicator LONGCROSS(const Indicator& x, Indicator::value_t y, const Indicator& n) {
    return LONGCROSS(x, CVAL(x, y), n);
}

inline Indicator LONGCROSS(Indicator::value_t x, const Indicator& y, int n = 3) {
    return LONGCROSS(CVAL(y, x), y, n);
}

inline Indicator LONGCROSS(Indicator::value_t x, const Indicator& y, const Indicator& n) {
    return LONGCROSS(CVAL(y, x), y, n);
}

inline Indicator LONGCROSS(Indicator::value_t x, Indicator::value_t y, int n = 3) {
    return LONGCROSS(CVAL(x), CVAL(y), n);
}

inline Indicator LONGCROSS(Indicator::value_t x, Indicator::value_t y, const Indicator& n) {
    return LONGCROSS(CVAL(x), CVAL(y), n);
}

inline void registerPart_LONGCROSS() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "LONGCROSS";
    meta.params.push_back(
      ParamMeta{.name = "x", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "y", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart("core.ind.longcross", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return LONGCROSS(std::get<Indicator>(pp.at("x").at(0)),
                                               std::get<Indicator>(pp.at("y").at(0)),
                                               static_cast<int>(sc.get<int64_t>("n")));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_LONGCROSS_H_ */
