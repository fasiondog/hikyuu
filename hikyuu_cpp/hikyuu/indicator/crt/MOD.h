/*
 * MOD.h
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_MOD_H_
#define INDICATOR_CRT_MOD_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Modulo after rounding
 * @details
 * <pre>
 * This function exists only for the TDX compatibility. In fact, the modulo of the indicators can be
 * done directly with the % operator
 * </pre>
 * @param ind1 indicator 1, it is rounded
 * @param ind2 indicator 2, it is rounded
 * @ingroup Indicator
 */

inline Indicator MOD(const Indicator& ind1, const Indicator& ind2) {
    return (ind1 % ind2);
}

inline Indicator MOD(const Indicator& ind1, Indicator::value_t ind2) {
    return ind1 % CVAL(ind1, ind2);
}

inline Indicator MOD(Indicator::value_t ind1, const Indicator& ind2) {
    return CVAL(ind2, ind1) % ind2;
}

inline Indicator MOD(Indicator::value_t ind1, Indicator::value_t ind2) {
    return CVAL(ind1) % CVAL(ind2);
}

inline void registerPart_MOD() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "MOD";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    registerIndicatorPart("core.ind.mod", meta,
                          [](const Parameter&, const PartParamMap& pp) -> Part {
                              return MOD(std::get<Indicator>(pp.at("ind1").at(0)),
                                         std::get<Indicator>(pp.at("ind2").at(0)));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_MOD_H_ */
