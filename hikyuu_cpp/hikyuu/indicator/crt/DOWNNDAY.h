/*
 * DOWNNDAY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DOWNNDAY_H_
#define INDICATOR_CRT_DOWNNDAY_H_

#include "EVERY.h"
#include "hikyuu/part/PartRegister.h"
#include "REF.h"

namespace hku {

/**
 * Consecutive falling periods; DOWNNDAY(CLOSE,M) means falling for M consecutive periods
 * @ingroup Indicator
 */
inline Indicator DOWNNDAY(const Indicator& ind, int n = 3) {
    Indicator result = EVERY(REF(ind, 1) > ind, n);
    result.name("DOWNNDAY");
    return result;
}

inline Indicator DOWNNDAY(const Indicator& ind, const Indicator& n) {
    Indicator result = EVERY(REF(ind, 1) > ind, n);
    result.name("DOWNNDAY");
    return result;
}

inline Indicator DOWNNDAY(const Indicator& ind, const IndParam& n) {
    Indicator result = EVERY(REF(ind, 1) > ind, n.get());
    result.name("DOWNNDAY");
    return result;
}

inline void registerPart_DOWNNDAY() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "DOWNNDAY";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "3"});
    registerIndicatorPart("core.ind.downnday", meta,
                          [](const Parameter& sc, const PartParamMap& pp) -> Part {
                              return DOWNNDAY(std::get<Indicator>(pp.at("ind").at(0)),
                                              static_cast<int>(sc.get<int64_t>("n")));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_DOWNNDAY_H_ */
