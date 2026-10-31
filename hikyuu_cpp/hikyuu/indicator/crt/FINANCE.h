/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

Indicator HKU_API FINANCE(int field_ix);
Indicator HKU_API FINANCE(const KData& k, int field_ix);

Indicator HKU_API FINANCE(const string& field_name);
Indicator HKU_API FINANCE(const KData& k, const string& field_name);

inline void registerPart_FINANCE() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "FINANCE";
    meta.params.push_back(ParamMeta{.name = "field_ix", .type = "int", .required = true});
    registerIndicatorPart("core.ind.finance", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return FINANCE(static_cast<int>(sc.get<int64_t>("field_ix")));
                          });
}

}  // namespace hku