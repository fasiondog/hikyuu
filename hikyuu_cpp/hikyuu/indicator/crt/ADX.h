#pragma once
#ifndef INDICATOR_CRT_ADX_H_
#define INDICATOR_CRT_ADX_H_

#include "../Indicator.h"

namespace hku {

/**
 * ADX average directional index
 * @details
 * <pre>
 * ADX (Average Directional Index) is a trend strength indicator, it does not distinguish the rising
 * or falling direction, it only judges whether there is a trend.
 *
 * The original formula of Wilder is used, with the period N = 14:
 *
 * 1. TR true range (the maximum of the three choices):
 *    TR = max(H-L, |H-C_prev|, |L-C_prev|)
 *
 * 2. DM directional movement:
 *    +DM = H - H_prev (if H > H_prev and the upward move > the downward move), otherwise 0
 *    -DM = L_prev - L (if L < L_prev and the downward move > the upward move), otherwise 0
 *
 * 3. Wilder smoothing (a simple average of N periods at the beginning, recursive afterwards):
 *    ATR_t = ATR_{t-1} × (N-1)/N + TR_t/N
 *    S+DM_t = S+DM_{t-1} × (N-1)/N + +DM_t/N
 *    S-DM_t = S-DM_{t-1} × (N-1)/N + -DM_t/N
 *
 * 4. ±DI directional indicator (percentage 0~100):
 *    +DI = (S+DM / ATR) × 100
 *    -DI = (S-DM / ATR) × 100
 *
 * 5. DX directional indicator:
 *    DX = |+DI - (-DI)| / (+DI + -DI) × 100
 *
 * 6. ADX average directional index (DX is smoothed by Wilder once more):
 *    ADX_t = ADX_{t-1} × (N-1)/N + DX_t/N
 *
 * Result set:
 * - 0: ADX itself (trend strength, value range 0~100)
 * - 1: +DI (upward directional line, bull power)
 * - 2: -DI (downward directional line, bear power)
 *
 * Judgment criteria:
 * - ADX >= 25: a clear one-sided trend exists (either rising or falling)
 * - ADX < 25: no trend, range-bound oscillation
 * - The larger the ADX value, the stronger the trend
 * </pre>
 * @param n calculation period, 14 by default
 * @ingroup Indicator
 */
Indicator HKU_API ADX(int n = 14);

/**
 * ADX average directional index
 * @param kdata the source data to be calculated
 * @param n calculation period, 14 by default
 * @ingroup Indicator
 */
Indicator HKU_API ADX(const KData& kdata, int n = 14);

}  // namespace hku

#endif /* INDICATOR_CRT_ADX_H_ */