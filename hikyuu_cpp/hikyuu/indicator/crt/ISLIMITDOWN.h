/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

#include "../Indicator.h"

namespace hku {

/**
 * @brief 判断股票是否跌停指标
 * @ingroup Indicator
 * @details 根据不同股票类型判断当日收盘价是否达到跌停板价格：
 * - A股普通股票：跌停幅度为10%
 * - 北交所股票：跌停幅度为30%
 * - 创业板/科创板股票：跌停幅度为20%
 * - ST股票跌停幅度为5%，但由于缺乏ST标识的历史日期信息，暂未处理
 *
 * 跌停判断逻辑：当日收盘价 <= 前一日收盘价 × (1 - 跌停幅度)
 *
 * 注意事项：
 * - 第一个K线数据由于缺少前一日数据，会被标记为discard（舍弃）
 * - 不支持的股票类型默认返回非跌停状态
 * - 计算结果为布尔值：1表示跌停，0表示非跌停
 *
 * <pre>
 * 示例：
 * @code
 * // 创建跌停判断指标
 * Indicator limit_down = ISLIMITDOWN();
 *
 * // 对指定K线数据进行判断
 * KData kdata = sm["sh000001"].getKData(KQuery(-100));
 * Indicator result = ISLIMITDOWN(kdata);
 *
 * // 获取判断结果
 * for (size_t i = 0; i < result.size(); ++i) {
 *     if (result[i] == 1.0) {
 *         cout << "第" << i << "天跌停" << endl;
 *     }
 * }
 * @endcode
 * </pre>
 *
 * @return Indicator 跌停判断指标实例
 */
Indicator HKU_API ISLIMITDOWN();

/**
 * @brief 判断指定K线数据中的股票是否跌停
 * @ingroup Indicator
 * @param k K线数据
 * @return Indicator 跌停判断指标实例
 * @see ISLIMITDOWN()
 */
Indicator HKU_API ISLIMITDOWN(const KData& k);

}  // namespace hku