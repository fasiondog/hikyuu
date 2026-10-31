/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

Indicator HKU_API RESULT(int result_ix);

inline Indicator RESULT(const Indicator& ind, int result_ix) {
    return RESULT(result_ix)(ind);
}

inline void registerPart_RESULT() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "RESULT";
    meta.params.push_back(ParamMeta{.name = "result_ix", .type = "int", .required = true});
    registerIndicatorPart("core.ind.result", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return RESULT(static_cast<int>(sc.get<int64_t>("result_ix")));
                          });
}

}  // namespace hku