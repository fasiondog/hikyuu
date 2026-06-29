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

    if (n == 1) {
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = 0.0;
        }
        return;
    }

    if (n == total - m_discard) {
        value_t pre_max = src[m_discard];
        value_t min_dd = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (src[i] > pre_max) {
                pre_max = src[i];
            }
            value_t dd = (src[i] >= pre_max || pre_max == 0.) ? 0.0 : (src[i] - pre_max) / pre_max;
            if (dd < min_dd) {
                min_dd = dd;
            }
            dst[i] = std::abs(min_dd * 100.0);
        }
        return;
    }

    value_t pre_max = src[m_discard];
    value_t min_dd = 0.0;
    for (size_t i = m_discard; i < m_discard + n; ++i) {
        if (src[i] > pre_max) {
            pre_max = src[i];
        }
        value_t dd = (src[i] >= pre_max || pre_max == 0.) ? 0.0 : (src[i] - pre_max) / pre_max;
        if (dd < min_dd) {
            min_dd = dd;
        }
        dst[i] = std::abs(min_dd * 100.0);
    }

    if (m_discard + n < total) {
        _increment_calculate(ind, m_discard + n);
    }
}

bool IMdd::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IMdd::min_increment_start() const {
    return getParam<int>("n");
}

void IMdd::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    // 标准最大回撤语义: dd_j = (run_max_j - src[j]) / run_max_j,
    // 其中 run_max_j = max(src[window_left..j]) 为到 j 为止的累计最大值。
    // 原实现错误地用窗口全局 max 作为所有点的回撤基准, 当窗口最高点出现在
    // 最低点之后时会引入未来数据(look-ahead bias), 高估回撤。
    // 此处放弃原 O(1) 快路径状态机(其 current_dd > window_max_dd 判断在标准
    // MDD 语义下原理性不成立, 因不同 j 的 dd 用不同的 run_max_j 基准),
    // 退化为每点 O(n) 暴力扫描, 用 run_max 基准保证正确性。
    for (size_t i = start_pos; i < total; ++i) {
        Indicator::value_t current_nav = src[i];
        if (std::isnan(current_nav) || current_nav <= 0.0) {
            // 无效点不写 dst[i], 保持原值, 与原语义一致
            continue;
        }

        size_t window_left = i + 1 - n;
        Indicator::value_t run_max = 0.0;  // 不用 src[window_left] 初始化, 避免 NaN 污染比较
        Indicator::value_t window_max_dd = 0.0;
        bool has_valid = false;
        for (size_t j = window_left; j <= i; ++j) {
            Indicator::value_t v = src[j];
            if (std::isnan(v) || v <= 0.0) {
                continue;
            }
            has_valid = true;
            if (v > run_max) {
                run_max = v;
            }
            // run_max 必然 > 0(数据约束为正), 除法安全
            Indicator::value_t dd = (run_max - v) / run_max;
            if (dd > window_max_dd) {
                window_max_dd = dd;
            }
        }

        if (has_valid) {
            dst[i] = window_max_dd * 100.0;
        }
    }
}

Indicator HKU_API MDD(int n) {
    IndicatorImpPtr p = make_shared<IMdd>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku