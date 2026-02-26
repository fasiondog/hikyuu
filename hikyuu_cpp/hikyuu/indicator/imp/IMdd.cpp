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

    Indicator::value_t window_max_val = 0.0;  // 当前窗口最大值
    size_t window_max_idx = 0;                // 当前窗口最大值的索引
    Indicator::value_t window_max_dd = 0.0;   // 当前窗口最大回撤

    for (size_t i = start_pos; i < total; ++i) {
        Indicator::value_t current_nav = src[i];

        // 处理无效值（NaN/非正数）
        if (std::isnan(current_nav) || current_nav <= 0) {
            // dst[i] = Null<Indicator::value_t>();
            window_max_val = 0.0;
            window_max_idx = i + 1;
            window_max_dd = 0.0;
            continue;
        }

        size_t window_left = i + 1 - n;

        bool need_rescan_max = false;
        // 若最大值索引被移出窗口 → 需要重新扫描窗口找最大值
        if (window_max_idx < window_left) {
            need_rescan_max = true;
        } else if (current_nav >= window_max_val) {
            // 若当前值大于等于最大值 → 直接更新最大值（无需扫描）
            window_max_val = current_nav;
            window_max_idx = i;
        }

        if (need_rescan_max) {
            window_max_val = current_nav;
            window_max_idx = i;
            for (size_t j = window_left; j < i; ++j) {
                if (src[j] > window_max_val) {
                    window_max_val = src[j];
                    window_max_idx = j;
                }
            }
        }

        bool need_rescan_dd = false;
        // 计算当前点的回撤
        Indicator::value_t current_dd = (window_max_val - current_nav) / window_max_val;

        // 若当前回撤更大 → 直接更新（推进式，无需扫描）
        if (current_dd > window_max_dd) {
            window_max_dd = current_dd;
        } else if (i >= n) {
            // 若窗口左边界移动，且移出的点是当前最大回撤的点 → 需重新扫描（极少触发）
            Indicator::value_t removed_dd = (window_max_val - src[i - n]) / window_max_val;
            if (removed_dd == window_max_dd) {
                need_rescan_dd = true;
            }
        }

        // 仅在必要时扫描窗口找最大回撤
        if (need_rescan_dd || need_rescan_max) {
            window_max_dd = 0.0;
            for (size_t j = window_left; j <= i; ++j) {
                Indicator::value_t dd = (window_max_val - src[j]) / window_max_val;
                if (dd > window_max_dd) {
                    window_max_dd = dd;
                }
            }
        }

        dst[i] = window_max_dd * 100.;
    }
}

Indicator HKU_API MDD(int n) {
    IndicatorImpPtr p = make_shared<IMdd>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku