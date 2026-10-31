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
 * Calculate the sample covariance
 * @param ind1 indicator 1
 * @param ind2 indicator 2
 * @param n the rolling window (>= 2); when it is 0 an expanding window is used, i.e. dst[i] is
 *          the sample cov over [discard(), i] (discard becomes 1, bar 0 is nan).
 * @param fill_null fill the missing dates with nan when the dates are aligned
 * @ingroup Indicator
 */
Indicator HKU_API COV(const Indicator& ind1, const Indicator& ind2, int n = 10,
                      bool fill_null = true);
Indicator HKU_API COV(const Indicator& ref_ind, int n = 10, bool fill_null = true);

inline void registerPart_COV() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "COV";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "10"});
    meta.params.push_back(ParamMeta{.name = "fill_null", .type = "bool", .default_value = "true"});
    registerIndicatorPart(
      "core.ind.cov", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return COV(std::get<Indicator>(pp.at("ind1").at(0)),
                     std::get<Indicator>(pp.at("ind2").at(0)),
                     static_cast<int>(sc.get<int64_t>("n")), sc.get<bool>("fill_null"));
      });
}

}  // namespace hku