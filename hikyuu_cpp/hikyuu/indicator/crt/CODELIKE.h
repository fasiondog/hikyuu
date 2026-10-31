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
 * @brief Return whether the security code matches the given pattern
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator HKU_API CODELIKE(const string& pattern);

/**
 * @brief Return whether the security code matches the given pattern
 * @param kdata K-line data
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator HKU_API CODELIKE(const KData& kdata, const string& pattern);

inline void registerPart_CODELIKE() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "CODELIKE";
    meta.params.push_back(ParamMeta{.name = "pattern", .type = "string", .required = true});
    registerIndicatorPart("core.ind.codelike", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return CODELIKE(sc.get<string>("pattern"));
                          });
}

}  // namespace hku
