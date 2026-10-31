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
 * Calculate the sample correlation coefficient and covariance. In the returned result sets, the
 * first is the correlation coefficient and the second is the covariance
 * @note Non-finite inputs (NaN or ±Inf) violate the "inf poisons downstream aggregates" contract;
 *       when any of varx / vary / cov is non-finite or the variances are non-positive, result(0)
 *       is Null (result(1) still follows raw cov arithmetic).
 * @param ind1 indicator 1
 * @param ind2 indicator 2
 * @param n the rolling window (>= 2); when it is 0 an expanding window is used, i.e. dst[i] is
 *          the Pearson corr & sample cov over [discard(), i] (discard becomes 1, bar 0 is nan).
 * @param fill_null fill the missing dates with nan when the dates are aligned
 * @ingroup Indicator
 */
Indicator HKU_API CORR(const Indicator& ind1, const Indicator& ind2, int n = 10,
                       bool fill_null = true);
Indicator HKU_API CORR(const Indicator& ref_ind, int n = 10, bool fill_null = true);

inline void registerPart_CORR() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "CORR";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "10"});
    meta.params.push_back(ParamMeta{.name = "fill_null", .type = "bool", .default_value = "true"});
    registerIndicatorPart(
      "core.ind.corr", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return CORR(std::get<Indicator>(pp.at("ind1").at(0)),
                      std::get<Indicator>(pp.at("ind2").at(0)),
                      static_cast<int>(sc.get<int64_t>("n")), sc.get<bool>("fill_null"));
      });
}

}  // namespace hku