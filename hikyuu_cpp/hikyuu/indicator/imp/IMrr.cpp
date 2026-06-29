/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMrr.h"

#include <vector>
#include <limits>

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

namespace {

// SWAG 滑动窗口最大盈利比率(MRR)的 O(N) 摊还算法。与 IMdd 对称。
// MRR 满足结合律: State = {MRR, MAX, MIN}, 两段合并 cross_mrr = B.MAX/A.MIN - 1
// (谷在A峰在B的跨界盈利, 与 MDD 的峰在A谷在B相反)。
// combined.MRR = max(A.MRR, B.MRR, cross_mrr)。双栈模拟队列, 冷启动 O(n)。

constexpr Indicator::value_t kPosInf = std::numeric_limits<Indicator::value_t>::infinity();
constexpr Indicator::value_t kNegInf = -std::numeric_limits<Indicator::value_t>::infinity();

struct MrrState {
    Indicator::value_t mrr;
    Indicator::value_t max_val;
    Indicator::value_t min_val;
};

const MrrState kIdent{0.0, kNegInf, kPosInf};

inline MrrState mrr_combine(const MrrState& a, const MrrState& b) {
    MrrState r;
    r.max_val = std::max(a.max_val, b.max_val);
    r.min_val = std::min(a.min_val, b.min_val);
    // 跨界盈利: 谷在 a(a.MIN), 峰在 b(b.MAX)
    Indicator::value_t cross = 0.0;
    if (a.min_val != kPosInf && b.max_val != kNegInf && a.min_val > 0.0) {
        cross = b.max_val / a.min_val - 1.0;
    }
    r.mrr = std::max(a.mrr, std::max(b.mrr, cross));
    return r;
}

inline MrrState mrr_single(Indicator::value_t v) {
    if (std::isnan(v) || v <= 0.0) return kIdent;
    return {0.0, v, v};
}

struct MrrElement {
    Indicator::value_t val;
    MrrState state;
};

class SwagMrr {
    std::vector<MrrElement> front_;
    std::vector<MrrElement> back_;

public:
    explicit SwagMrr(size_t n) {
        front_.reserve(n + 1);
        back_.reserve(n + 1);
    }
    void push(Indicator::value_t v) {
        MrrState s = mrr_single(v);
        if (!back_.empty()) s = mrr_combine(back_.back().state, s);
        back_.push_back({v, s});
    }
    void pop() {
        if (front_.empty()) {
            while (!back_.empty()) {
                Indicator::value_t v = back_.back().val;
                back_.pop_back();
                MrrState s = mrr_single(v);
                if (!front_.empty()) s = mrr_combine(s, front_.back().state);
                front_.push_back({v, s});
            }
        }
        if (!front_.empty()) front_.pop_back();
    }
    Indicator::value_t query_mrr() const {
        MrrState sf = front_.empty() ? kIdent : front_.back().state;
        MrrState sb = back_.empty() ? kIdent : back_.back().state;
        return mrr_combine(sf, sb).mrr;
    }
    size_t size() const { return front_.size() + back_.size(); }
};

}  // namespace

void IMrr::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    // 标准最大盈利比率: rr_j = src[j]/run_min_j - 1, run_min_j = min(src[window_left..j])
    // (到 j 为止累计最小, 路径依赖)。原实现用窗口全局 min 引入 look-ahead bias,
    // 且 O(1) 状态机无法局部修补。采用 SWAG O(N) 摊还算法(与 IMdd 对称),
    // 冷启动 O(n) 重建窗口历史, 之后每步 O(1) 摊还。
    SwagMrr swag(n);
    size_t init_left = (start_pos + 1 > n) ? (start_pos + 1 - n) : 0;
    for (size_t j = init_left; j < start_pos; ++j) {
        swag.push(src[j]);
    }
    for (size_t i = start_pos; i < total; ++i) {
        swag.push(src[i]);
        while (swag.size() > n) {
            swag.pop();
        }
        dst[i] = swag.query_mrr() * 100.0;
    }
}

Indicator HKU_API MRR(int n) {
    IndicatorImpPtr p = make_shared<IMrr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku