/*
 * MAX.h
 *
 *  Created on: 2019-4-8
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_MAX_H_
#define INDICATOR_CRT_MAX_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

#if defined(MAX)
#undef MAX
#endif

/**
 * Calculate the maximum
 * @details
 * <pre>
 * Usage: MAX(A,B) returns the larger one of A and B
 * For example: MAX(CLOSE-OPEN,0) returns their difference if the close price is greater than the
 * open price, otherwise it returns 0
 * </pre>
 * @ingroup Indicator
 */
inline Indicator MAX(const Indicator& ind1, const Indicator& ind2) {
    Indicator result = IF(ind1 > ind2, ind1, ind2);
    result.name("MAX");
    return result;
}

inline Indicator MAX(const Indicator& ind, Indicator::value_t val) {
    Indicator result = IF(ind > val, ind, val);
    result.name("MAX");
    return result;
}

inline Indicator MAX(Indicator::value_t val, const Indicator& ind) {
    Indicator result = IF(val > ind, val, ind);
    result.name("MAX");
    return result;
}

inline void registerPart_MAX() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "MAX";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.max", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return MAX(std::get<Indicator>(pp.at("ind1").at(0)),
                                         std::get<Indicator>(pp.at("ind2").at(0)));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_MAX_H_ */
