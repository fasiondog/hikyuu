/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMdd.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMdd)
#endif

namespace hku {

IMdd::IMdd() : IndicatorImp("MDD", 1) {
    setParam<int>("n", 0);
}

IMdd::~IMdd() {}

void IMdd::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

/*
 * 纯推进式滑动窗口最大回撤（C风格指针接口）
 * @param nav_series 输入：净值序列指针（非空，长度=total）
 * @param total 输入：净值序列总长度
 * @param window_size 输入：滑动窗口大小（≥1）
 * @param output 输出：最大回撤结果数组（非空，长度=total，由调用方分配内存）
 * @throws std::invalid_argument 入参非法时抛出异常
 * @note 推进式逻辑，仅在窗口最大值更新时遍历窗口，其余逐点推进
 */
void calculate_sliding_window_max_drawdown(const Indicator::value_t* nav_series, size_t total,
                                           size_t window_size, Indicator::value_t* output) {
    // 被调用时已保证传入参数有效，total > 0 且 window_size > 0, output 已分配足够内存
    // 推进式核心变量（全程仅维护这几个变量，无队列）
    Indicator::value_t window_max_val = 0.0;  // 当前窗口最大值
    size_t window_max_idx = 0;                // 当前窗口最大值的索引
    Indicator::value_t window_max_dd = 0.0;   // 当前窗口最大回撤
    size_t window_left = 0;                   // 窗口左边界

    // 单循环逐点推进（核心：全程只有这一个for循环）
    for (size_t i = 0; i < total; ++i) {
        Indicator::value_t current_nav = nav_series[i];

        // 处理无效值（NaN/非正数）
        if (std::isnan(current_nav) || current_nav <= 0) {
            // output[i] = std::numeric_limits<Indicator::value_t>::quiet_NaN();
            // 重置推进状态
            window_max_val = 0.0;
            window_max_idx = i + 1;
            window_max_dd = 0.0;
            window_left = i + 1;
            continue;
        }

        window_left = (i >= window_size) ? (i - window_size + 1) : 0;

        bool need_rescan_max = false;
        // 若最大值索引被移出窗口 → 需要重新扫描窗口找最大值
        if (window_max_idx < window_left) {
            need_rescan_max = true;
        }
        // 若当前值大于等于最大值 → 直接更新最大值（无需扫描）
        else if (current_nav >= window_max_val) {
            window_max_val = current_nav;
            window_max_idx = i;
        }

        // 仅在必要时扫描窗口找最大值（推进式的核心：极少触发）
        if (need_rescan_max) {
            window_max_val = current_nav;
            window_max_idx = i;
            for (size_t j = window_left; j < i; ++j) {
                if (nav_series[j] > window_max_val) {
                    window_max_val = nav_series[j];
                    window_max_idx = j;
                }
            }
        }

        // -------------------------- 步骤3：推进式更新窗口最大回撤 --------------------------
        bool need_rescan_dd = false;
        // 计算当前点的回撤
        Indicator::value_t current_dd = (window_max_val - current_nav) / window_max_val;

        // 若当前回撤更大 → 直接更新（推进式，无需扫描）
        if (current_dd > window_max_dd) {
            window_max_dd = current_dd;
        }
        // 若窗口左边界移动，且移出的点是当前最大回撤的点 → 需重新扫描（极少触发）
        else if (i >= window_size) {
            Indicator::value_t removed_dd =
              (window_max_val - nav_series[i - window_size]) / window_max_val;
            if (removed_dd == window_max_dd) {
                need_rescan_dd = true;
            }
        }

        // 仅在必要时扫描窗口找最大回撤（推进式的核心：极少触发）
        if (need_rescan_dd || need_rescan_max) {
            window_max_dd = 0.0;
            for (size_t j = window_left; j <= i; ++j) {
                Indicator::value_t dd = (window_max_val - nav_series[j]) / window_max_val;
                if (dd > window_max_dd) {
                    window_max_dd = dd;
                }
            }
        }

        // -------------------------- 步骤4：写入结果 --------------------------
        output[i] = window_max_dd * 100.;
    }
}

void IMdd::_calculate(const Indicator& ind) {
    m_discard = ind.discard();
    size_t total = ind.size();
    HKU_IF_RETURN(m_discard >= total, void());

    auto const* src = ind.data();
    auto* dst = this->data();

    size_t n = static_cast<size_t>(getParam<int>("n"));
    if (n == 0 || n > total - m_discard) {
        n = total - m_discard;
    }

    if (n == total - m_discard) {
        value_t pre_max = src[m_discard];
        value_t min_dd = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (src[i] > pre_max) {
                pre_max = src[i];
            }
            value_t dd =
              (src[i] >= pre_max || pre_max == 0.) ? 0.0 : (src[i] / pre_max - 1.0) * 100.0;
            if (dd < min_dd) {
                min_dd = dd;
            }
            dst[i] = std::abs(min_dd);
        }
        return;
    }

    _increment_calculate(ind, m_discard);
}

bool IMdd::supportIncrementCalculate() const {
    return getParam<int>("n") > 0;
}

size_t IMdd::min_increment_start() const {
    return getParam<int>("n") - 1;
}

void IMdd::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    calculate_sliding_window_max_drawdown(src + start_pos, total - start_pos, n, dst + start_pos);
}

Indicator HKU_API MDD(int n) {
    IndicatorImpPtr p = make_shared<IMdd>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku