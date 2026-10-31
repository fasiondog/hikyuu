/*
 * MIN.h
 *
 *  Created on: 2019-4-8
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_MIN_H_
#define INDICATOR_CRT_MIN_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

#if defined(MIN)
#undef MIN
#endif

/**
 * Calculate the minimum
 * @details
 * <pre>
 * Usage: MIN(A,B) returns the smaller one of A and B
 * For example: MIN(CLOSE,OPEN) returns the smaller one of the open price and the close price
 * </pre>
 * @ingroup Indicator
 */
inline Indicator MIN(const Indicator& ind1, const Indicator& ind2) {
    Indicator result = IF(ind1 < ind2, ind1, ind2);
    result.name("MIN");
    return result;
}

inline Indicator MIN(const Indicator& ind, Indicator::value_t val) {
    Indicator result = IF(ind < val, ind, val);
    result.name("MIN");
    return result;
}

inline Indicator MIN(Indicator::value_t val, const Indicator& ind) {
    Indicator result = IF(val < ind, val, ind);
    result.name("MIN");
    return result;
}

inline void registerPart_MIN() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "MIN";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.min", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return MIN(std::get<Indicator>(pp.at("ind1").at(0)),
                                         std::get<Indicator>(pp.at("ind2").at(0)));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_MIN_H_ */
