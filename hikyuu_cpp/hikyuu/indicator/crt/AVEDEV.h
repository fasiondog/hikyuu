/*
 * AVEDEV.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2015-5-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_AVEDEV_H_
#define INDICATOR_CRT_AVEDEV_H_

#include "MA.h"
#include "hikyuu/part/PartRegister.h"
#include "ABS.h"

namespace hku {

/**
 * Mean absolute deviation, it gives the N-day mean absolute deviation of X
 * @param ind the data to be calculated
 * @param n time window
 * @ingroup Indicator
 */
inline Indicator AVEDEV(const Indicator& ind, int n = 22) {
    Indicator result = ABS(ind - MA(ind, n)) / n;
    result.name("AVEDEV");
    return result;
}

inline Indicator AVEDEV(const Indicator& ind, const Indicator& n) {
    Indicator result = ABS(ind - MA(ind, n)) / n;
    result.name("AVEDEV");
    return result;
}

inline Indicator AVEDEV(const Indicator& ind, const IndParam& n) {
    Indicator result = ABS(ind - MA(ind, n)) / n.get();
    result.name("AVEDEV");
    return result;
}

inline void registerPart_AVEDEV() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "AVEDEV";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "22"});
    registerIndicatorPart("core.ind.avedev", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return AVEDEV(std::get<Indicator>(pp.at("ind").at(0)),
                                            static_cast<int>(sc.get<int64_t>("n")));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_AVEDEV_H_ */
