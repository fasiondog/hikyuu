/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Calculate the Beta coefficient, such as measuring the sensitivity between the asset return and
 * the market return

 * Beta = Cov(stock_return, market_return) / Var(market_return)
 *
 * @note Non-finite inputs (NaN or ±Inf) that make vary or cov non-finite yield Null output,
 *       matching the library's "no non-finite value flows downstream" contract.
 *
 * @note BETA itself does not perform the return conversion (pct_change) on the input data,

 *       the input indicators should be the already calculated return data.

 * @param ind1 the input indicator, such as the stock return indicator

 * @param ind2 the reference indicator, such as the market return indicator

 * @param n the rolling window (>= 2); when it is 0 an expanding window is used, i.e. dst[i] is
 *          the beta over [discard(), i] (discard becomes 1, bar 0 is nan).

 * @param fill_null fill the missing dates with nan when the dates are aligned

 * @ingroup Indicator
 */
Indicator HKU_API BETA(const Indicator& ind1, const Indicator& ind2, int n = 10,
                       bool fill_null = true);
Indicator HKU_API BETA(const Indicator& ref_ind, int n = 10, bool fill_null = true);

inline void registerPart_BETA() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "BETA";
    meta.params.push_back(
      ParamMeta{.name = "ind1", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "ind2", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "10"});
    meta.params.push_back(ParamMeta{.name = "fill_null", .type = "bool", .default_value = "true"});
    registerIndicatorPart(
      "core.ind.beta", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return BETA(std::get<Indicator>(pp.at("ind1").at(0)),
                      std::get<Indicator>(pp.at("ind2").at(0)),
                      static_cast<int>(sc.get<int64_t>("n")), sc.get<bool>("fill_null"));
      });
}

}  // namespace hku