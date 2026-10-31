#pragma once
#include <hikyuu/indicator/Indicator.h>
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * @brief Average directional index (ADX2) - using the EMA smoothing way
 *
 * ADX2 is a trend strength indicator, it does not distinguish the rising or falling direction, it
 * only judges whether there is a trend.
 * Unlike ADX, it uses EMA (exponential moving average) instead of the Wilder smoothing.
 *
 * Result set:
 * - result(0): ADX itself (trend strength, value range 0~100)
 * - result(1): +DI (upward directional line, bull power)
 * - result(2): -DI (downward directional line, bear power)
 *
 * Judgment criteria:
 * - ADX >= 25: a clear one-sided trend exists (either rising or falling)
 * - ADX < 25: no trend, range-bound oscillation
 * - The larger the ADX value, the stronger the trend
 *
 * @param kdata the source data to be calculated
 * @param n calculation period, 14 by default, it must be an integer greater than 1
 * @return the Indicator with three result sets
 */
Indicator HKU_API ADX2(const KData& kdata, int n = 14);

/**
 * @brief Average directional index (ADX2) - using the EMA smoothing way
 *
 * Create an ADX2 indicator calculator, the context needs to be set through setContext
 *
 * @param n calculation period, 14 by default, it must be an integer greater than 1
 * @return the ADX2 indicator calculator
 */
Indicator HKU_API ADX2(int n = 14);

inline void registerPart_ADX2() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "ADX2";
    meta.params.push_back(ParamMeta{.name = "kdata", .type = "KData", .required = true});
    meta.params.push_back(ParamMeta{.name = "n", .type = "int", .default_value = "14"});
    registerIndicatorPart(
      "core.ind.adx2", meta, [](const Parameter& sc, const PartParamMap&) -> Part {
          return ADX2(sc.get<KData>("kdata"), static_cast<int>(sc.get<int64_t>("n")));
      });
}

}  // namespace hku