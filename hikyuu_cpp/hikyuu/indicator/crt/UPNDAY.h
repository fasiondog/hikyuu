/*
 * UPNDAY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_UPNDAY_H_
#define INDICATOR_CRT_UPNDAY_H_

#include "EVERY.h"
#include "hikyuu/part/PartRegister.h"
#include "REF.h"

namespace hku {

/**
 * Consecutive rising periods; UPNDAY(CLOSE,M) means rising for M consecutive periods
 * @ingroup Indicator
 */
inline Indicator UPNDAY(const Indicator& ind, int n = 3) {
    Indicator result = EVERY(ind > REF(ind, 1), n);
    result.name("UPDAY");
    return result;
}

inline Indicator UPNDAY(const Indicator& ind, const IndParam& n) {
    Indicator result = EVERY(ind > REF(ind, 1), n);
    result.name("UPDAY");
    return result;
}

inline Indicator UPNDAY(const Indicator& ind, const Indicator& n) {
    Indicator result = EVERY(ind > REF(ind, 1), n);
    result.name("UPDAY");
    return result;
}

inline void registerPart_UPNDAY() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "UPNDAY";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart("core.ind.upnday", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return UPNDAY(std::get<Indicator>(pp.at("ind").at(0)),
                                            static_cast<int>(sc.get<int64_t>("n")));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_UPNDAY_H_ */
