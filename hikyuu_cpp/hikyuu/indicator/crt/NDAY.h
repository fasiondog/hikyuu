/*
 * NDAY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_NDAY_H_
#define INDICATOR_CRT_NDAY_H_

#include "EVERY.h"
#include "hikyuu/part/PartRegister.h"
#include "REF.h"

namespace hku {

/**
 * Consecutive greater; NDAY(X,Y,N) means the condition X>Y exists persistently for N periods
 * @ingroup Indicator
 */
inline Indicator NDAY(const Indicator& x, const Indicator& y, int n = 3) {
    Indicator result = EVERY(x > y, n);
    result.name("NDAY");
    return result;
}

inline Indicator NDAY(const Indicator& x, const Indicator& y, const Indicator& n) {
    Indicator result = EVERY(x > y, n);
    result.name("NDAY");
    return result;
}

inline Indicator NDAY(const Indicator& x, const Indicator& y, const IndParam& n) {
    Indicator result = EVERY(x > y, n);
    result.name("NDAY");
    return result;
}

inline void registerPart_NDAY() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "NDAY";
    meta.params.push_back(
      ParamMeta{.name = "x", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "y", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart(
      "core.ind.nday", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return NDAY(std::get<Indicator>(pp.at("x").at(0)), std::get<Indicator>(pp.at("y").at(0)),
                      static_cast<int>(sc.get<int64_t>("n")));
      });
}

}  // namespace hku

#endif /* INDICATOR_CRT_NDAY_H_ */
