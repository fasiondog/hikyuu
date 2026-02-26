/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMrr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMrr)
#endif

namespace hku {

IMrr::IMrr() : IndicatorImp("MRR", 1) {
    setParam<int>("n", 0);
}

IMrr::~IMrr() {}

void IMrr::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IMrr::_calculate(const Indicator& ind) {
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
        value_t pre_min = src[m_discard];
        value_t max_rr = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (src[i] < pre_min || pre_min == 0.) {
                pre_min = src[i];
            }
            value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
            if (rr > max_rr) {
                max_rr = rr;
            }
            dst[i] = max_rr * 100.0;
        }
        return;
    }

    value_t pre_min = src[m_discard];
    value_t max_rr = 0.0;
    for (size_t i = m_discard; i < m_discard + n; ++i) {
        if (src[i] < pre_min || pre_min == 0.) {
            pre_min = src[i];
        }
        value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
        if (rr > max_rr) {
            max_rr = rr;
        }
        dst[i] = max_rr * 100.0;
    }

    if (m_discard + n < total) {
        _increment_calculate(ind, m_discard + n);
    }
}

bool IMrr::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IMrr::min_increment_start() const {
    return getParam<int>("n");
}

void IMrr::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    Indicator::value_t window_min_val = 0.0;  // 当前窗口最小值
    size_t window_min_idx = 0;                // 当前窗口最小值的索引
    Indicator::value_t window_max_rr = 0.0;   // 当前窗口最大盈利比率
    size_t window_left = start_pos + 1 - n;   // 窗口左边界

    for (size_t i = start_pos; i < total; ++i) {
        Indicator::value_t current_price = src[i];

        // 处理无效值（NaN/非正数）
        if (std::isnan(current_price) || current_price <= 0) {
            window_min_val = 0.0;
            window_min_idx = i + 1;
            window_max_rr = 0.0;
            window_left = i + 1;
            continue;
        }

        window_left = i + 1 - n;

        bool need_rescan_min = false;
        // 若最小值索引被移出窗口 → 需要重新扫描窗口找最小值
        if (window_min_idx < window_left) {
            need_rescan_min = true;
        } else if (current_price <= window_min_val || window_min_val == 0.) {
            // 若当前值小于等于最小值 → 直接更新最小值（无需扫描）
            window_min_val = current_price;
            window_min_idx = i;
        }

        if (need_rescan_min) {
            window_min_val = current_price;
            window_min_idx = i;
            for (size_t j = window_left; j < i; ++j) {
                if (src[j] < window_min_val || window_min_val == 0.) {
                    window_min_val = src[j];
                    window_min_idx = j;
                }
            }
        }

        bool need_rescan_rr = false;
        // 计算当前点的盈利比率
        Indicator::value_t current_rr = (window_min_val == 0.) ? 0.0 : (current_price / window_min_val - 1.0);

        // 若当前盈利比率更大 → 直接更新（推进式，无需扫描）
        if (current_rr > window_max_rr) {
            window_max_rr = current_rr;
        } else if (i >= n) {
            // 若窗口左边界移动，且移出的点是当前最大盈利比率的点 → 需重新扫描（极少触发）
            Indicator::value_t removed_rr = (window_min_val == 0.) ? 0.0 : (src[i - n] / window_min_val - 1.0);
            if (removed_rr == window_max_rr) {
                need_rescan_rr = true;
            }
        }

        // 仅在必要时扫描窗口找最大盈利比率
        if (need_rescan_rr || need_rescan_min) {
            window_max_rr = 0.0;
            for (size_t j = window_left; j <= i; ++j) {
                Indicator::value_t rr = (window_min_val == 0.) ? 0.0 : (src[j] / window_min_val - 1.0);
                if (rr > window_max_rr) {
                    window_max_rr = rr;
                }
            }
        }

        dst[i] = window_max_rr * 100.;
    }
}

Indicator HKU_API MRR(int n) {
    IndicatorImpPtr p = make_shared<IMrr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku