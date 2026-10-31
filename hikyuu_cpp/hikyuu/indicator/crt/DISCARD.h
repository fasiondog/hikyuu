/*
 * COS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DISCARD_H_
#define INDICATOR_CRT_DISCARD_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/** Set the discard value in the way of an indicator formula */
Indicator HKU_API DISCARD(int discard);

inline Indicator DISCARD(const Indicator& ind, int discard) {
    return DISCARD(discard)(ind);
}

inline void registerPart_DISCARD() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "DISCARD";
    meta.params.push_back(ParamMeta{.name = "discard", .type = "int", .required = true});
    registerIndicatorPart("core.ind.discard", meta,
                          [](const Parameter &sc, const PartParamMap &) -> Part {
                              return DISCARD(static_cast<int>(sc.get<int64_t>("discard")));
                          });
}

}  // namespace hku

#endif /* INDICATOR_CRT_DISCARD_H_ */
