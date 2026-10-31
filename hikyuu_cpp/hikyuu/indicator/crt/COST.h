/*
 * COST.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-19
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_COST_H_
#define INDICATOR_CRT_COST_H_

#include "KDATA.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Cost distribution
 * @details
 * <pre>
 * Usage: COST(k, X) means what the price is when X% of the positions are profitable
 * For example: COST(k, 10) means what the price is when 10% of the positions are profitable, i.e.
 * 10% of the positions are below that price,
 *       and the remaining 90% are above that price and are trapped. This function is valid for the
 *       daily analysis period only
 * </pre>
 * @param k the associated K-line data
 * @param x the X% profitable positions
 * @ingroup Indicator
 */
Indicator HKU_API COST(const KData& k, double x = 10.0);
Indicator HKU_API COST(double x = 10.0);

inline void registerPart_COST() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "COST";
    meta.params.push_back(ParamMeta{.name = "k", .type = "KData", .required = true});
    meta.params.push_back(ParamMeta{.name = "x", .type = "double", .default_value = "10.0"});
    registerIndicatorPart("core.ind.cost", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return COST(sc.get<KData>("k"), sc.get<double>("x"));
                          });
}

}  // namespace hku
#endif /* INDICATOR_CRT_COST_H_ */
