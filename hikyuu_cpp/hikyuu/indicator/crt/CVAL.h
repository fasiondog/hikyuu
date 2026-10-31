/*
 * CVAL.h
 *
 *  Created on: 2017-6-25
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_CVAL_H_
#define INDICATOR_CRT_CVAL_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Create a constant indicator with the given length

 * @param value the constant

 * @param discard the number to discard, 0 by default

 * @ingroup Indicator
 */
Indicator HKU_API CVAL(double value, size_t discard = 0);

/**
 * Create a constant indicator whose length is the same as the input ind and whose value is fixed to
 * the given value

 * @param ind the data to be calculated

 * @param value the constant

 * @param discard the number to discard, 0 by default

 * @ingroup Indicator
 */
Indicator HKU_API CVAL(const Indicator& ind, double value = 0.0, int discard = 0);

inline void registerPart_CVAL() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "CVAL";
    meta.params.push_back(ParamMeta{.name = "value", .type = "double", .required = true});
    meta.params.push_back(ParamMeta{.name = "discard", .type = "int", .default_value = "0"});
    registerIndicatorPart(
      "core.ind.cval", meta, [](const Parameter& sc, const PartParamMap&) -> Part {
          return CVAL(sc.get<double>("value"), static_cast<size_t>(sc.get<int64_t>("discard")));
      });
}

}  // namespace hku

#endif /* INDICATOR_CRT_CVAL_H_ */
