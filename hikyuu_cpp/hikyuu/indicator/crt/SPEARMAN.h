/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Spearman correlation coefficient
 * @note Non-finite inputs (NaN or ±Inf) are excluded from the cross-section, aligned with the
 *       library's "ignore missing points" mean-class convention (AGENTS.md L247/L249).
 * @param ind indicator 1
 * @param ref_ind indicator 2
 * @param n the rolling window (>= 2); when it is 0 an expanding window is used, i.e. dst[i] is the
 *          rank corr over [max(ind.discard(), ref.discard()), i] (discard becomes 1, bar 0 is nan)
 * @param fill_null fill the missing values
 * @ingroup Indicator
 */
Indicator HKU_API SPEARMAN(const Indicator& ind, const Indicator& ref_ind, int n = 0,
                           bool fill_null = true);
Indicator HKU_API SPEARMAN(const Indicator& ref_ind, int n = 0, bool fill_null = true);

inline void registerPart_SPEARMAN() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "SPEARMAN";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ref_ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "0"});
    meta.params.push_back(ParamMeta{.name = "fill_null", .type = "bool", .default_value = "true"});
    registerIndicatorPart(
      "core.ind.spearman", meta, [](const Parameter &sc, const PartParamMap &pp) -> Part {
          return SPEARMAN(std::get<Indicator>(pp.at("ind").at(0)),
                          std::get<Indicator>(pp.at("ref_ind").at(0)),
                          static_cast<int>(sc.get<int64_t>("n")), sc.get<bool>("fill_null"));
      });
}

}  // namespace hku