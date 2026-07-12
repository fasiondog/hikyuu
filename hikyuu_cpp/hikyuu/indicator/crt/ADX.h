#pragma once
#ifndef INDICATOR_CRT_ADX_H_
#define INDICATOR_CRT_ADX_H_

#include "../Indicator.h"

namespace hku {

/**
 * ADX平均趋向指数
 * @details
 * <pre>
 * ADX（Average Directional Index 平均趋向指数）属于趋势强度指标，不分辨涨跌方向，只判断有没有趋势。
 *
 * 采用威尔德（Wilder）原始公式，周期N=14：
 *
 * 1. TR 真实波幅（三选一最大值）：
 *    TR = max(H-L, |H-C_prev|, |L-C_prev|)
 *
 * 2. 动向波动 DM：
 *    +DM = H - H_prev（如果H>H_prev且上攻幅度>下探幅度），否则为0
 *    -DM = L_prev - L（如果L<L_prev且下探幅度>上攻幅度），否则为0
 *
 * 3. Wilder平滑（初始为N周期简单平均，后续递归）：
 *    ATR_t = ATR_{t-1} × (N-1)/N + TR_t/N
 *    S+DM_t = S+DM_{t-1} × (N-1)/N + +DM_t/N
 *    S-DM_t = S-DM_{t-1} × (N-1)/N + -DM_t/N
 *
 * 4. ±DI 动向指数（百分比0~100）：
 *    +DI = (S+DM / ATR) × 100
 *    -DI = (S-DM / ATR) × 100
 *
 * 5. DX 动向指数：
 *    DX = |+DI - (-DI)| / (+DI + -DI) × 100
 *
 * 6. ADX 平均动向指数（DX再做一次Wilder平滑）：
 *    ADX_t = ADX_{t-1} × (N-1)/N + DX_t/N
 *
 * 结果集：
 * - 0: ADX本身（趋势强度，值域0~100）
 * - 1: +DI（上升动向线，多头力量）
 * - 2: -DI（下降动向线，空头力量）
 *
 * 判断标准：
 * - ADX ≥ 25：存在清晰单边趋势（上涨/下跌都行）
 * - ADX ＜ 25：无趋势，箱体震荡
 * - ADX数值越大，趋势越猛
 * </pre>
 * @param n 计算周期，默认14
 * @ingroup Indicator
 */
Indicator HKU_API ADX(int n = 14);

/**
 * ADX平均趋向指数
 * @param kdata 待计算的源数据
 * @param n 计算周期，默认14
 * @ingroup Indicator
 */
Indicator HKU_API ADX(const KData& kdata, int n = 14);

}  // namespace hku

#endif /* INDICATOR_CRT_ADX_H_ */