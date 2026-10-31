/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * @brief Return whether the security belongs to a certain block

 * @param category the given block category

 * @param name block name

 * @return Indicator
 */
Indicator HKU_API INBLOCK(const string& category, const string& name);

/**
 * @brief Return whether the security belongs to a certain block

 * @param kdata K-line data

 * @param category the given block category

 * @param name block name

 * @return Indicator
 */
Indicator HKU_API INBLOCK(const KData& kdata, const string& category, const string& name);

inline void registerPart_INBLOCK() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "INBLOCK";
    meta.params.push_back(ParamMeta{.name = "category", .type = "string", .required = true});
    meta.params.push_back(ParamMeta{.name = "name", .type = "string", .required = true});
    registerIndicatorPart("core.ind.inblock", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return INBLOCK(sc.get<string>("category"), sc.get<string>("name"));
                          });
}

}  // namespace hku