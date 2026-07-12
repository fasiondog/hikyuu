#pragma once
#include <hikyuu/indicator/Indicator.h>

namespace hku {

/**
 * @brief 平均趋向指数(ADX2) - 使用EMA平滑方式
 *
 * ADX2属于趋势强度指标，不分辨涨跌方向，只判断有没有趋势。
 * 与ADX的区别在于使用EMA（指数移动平均）而非Wilder平滑。
 *
 * 结果集：
 * - result(0): ADX本身（趋势强度，值域0~100）
 * - result(1): +DI（上升动向线，多头力量）
 * - result(2): -DI（下降动向线，空头力量）
 *
 * 判断标准：
 * - ADX >= 25：存在清晰单边趋势（上涨/下跌都行）
 * - ADX < 25：无趋势，箱体震荡
 * - ADX数值越大，趋势越猛
 *
 * @param kdata 待计算的源数据
 * @param n 计算周期，默认14，必须为大于1的整数
 * @return 具有三个结果集的 Indicator
 */
Indicator HKU_API ADX2(const KData& kdata, int n = 14);

/**
 * @brief 平均趋向指数(ADX2) - 使用EMA平滑方式
 *
 * 创建ADX2指标计算器，需通过setContext设置上下文
 *
 * @param n 计算周期，默认14，必须为大于1的整数
 * @return ADX2指标计算器
 */
Indicator HKU_API ADX2(int n = 14);

}  // namespace hku