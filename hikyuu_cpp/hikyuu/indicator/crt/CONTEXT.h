/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Independent context indicator
 * @param ind the indicator to be wrapped
 * @param fill_null whether to fill the null values, false by default
 * @param use_self_ktype use the K-line type of its own independent context, otherwise the K-line
 *                        type of the calculation context is used
 * @param use_self_recover_type use the adjustment type of its own independent context, otherwise
 *                              the adjustment type of the calculation context is used
 * @ingroup Indicator
 */
Indicator HKU_API CONTEXT(const Indicator& ind, bool fill_null = false, bool use_self_ktype = false,
                          bool use_self_recover_type = false);
Indicator HKU_API CONTEXT(bool fill_null = false, bool use_self_ktype = false,
                          bool use_self_recover_type = false);

/**
 * @brief Set an independent context for the indicator by the given stock
 * @param ind the input indicator formula; it is ignored if it carries a context itself
 * @param stk the given stock
 * @param fill_null whether to fill the null values, false by default
 * @return Indicator
 */
Indicator HKU_API CONTEXT(const Indicator& ind, const Stock& stk, bool fill_null = false);

/**
 * Get the indicator context
 * @note The Indicator::getContext() method gets the current context, but for the CONTEXT
 * independent context indicator its given independent context cannot be got by that method, it
 * must be got with this method.
 * Once this indicator participates in the calculation as a formula, its context may change, but
 * its stock remains unchanged, only the query range changes
 * @param ind
 * @return KData
 */
KData HKU_API CONTEXT_K(const Indicator& ind);

/**
 * @brief Judge whether the indicator is an independent context indicator
 * @param ind
 * @return true
 * @return false
 */
bool HKU_API is_standalone_context(const Indicator& ind);

inline void registerPart_CONTEXT() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "CONTEXT";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "fill_null", .type = "bool", .default_value = "false"});
    meta.params.push_back(
      ParamMeta{.name = "use_self_ktype", .type = "bool", .default_value = "false"});
    meta.params.push_back(
      ParamMeta{.name = "use_self_recover_type", .type = "bool", .default_value = "false"});
    registerIndicatorPart(
      "core.ind.context", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return CONTEXT(std::get<Indicator>(pp.at("ind").at(0)), sc.get<bool>("fill_null"),
                         sc.get<bool>("use_self_ktype"), sc.get<bool>("use_self_recover_type"));
      });
}

}  // namespace hku