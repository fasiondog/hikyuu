/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * @brief Return whether the security name matches the given pattern
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator HKU_API NAMELIKE(const string& pattern);

/**
 * @brief Return whether the security name matches the given pattern
 * @param kdata K-line data
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator HKU_API NAMELIKE(const KData& kdata, const string& pattern);

inline void registerPart_NAMELIKE() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "NAMELIKE";
    meta.params.push_back(ParamMeta{.name = "pattern", .type = "string", .required = true});
    registerIndicatorPart("core.ind.namelike", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return NAMELIKE(sc.get<string>("pattern"));
                          });
}

}  // namespace hku
