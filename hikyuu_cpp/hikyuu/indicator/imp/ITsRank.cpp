/*
 * ITsRank.cpp
 *
 *  Created on: 2026-6-9
 *      Author: fasiondog
 */

#include <climits>
#include "ITsRank.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ITsRank)
#endif

namespace hku {

ITsRank::ITsRank() : IndicatorImp("TS_RANK", 1) {
    setParam<int>("n", 20);
}

ITsRank::~ITsRank() {}

void ITsRank::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") > 0 && getParam<int>("n") < INT_MAX);
    }
}

void ITsRank::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total || ind.discard() >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    global_parallel_for_index_void(m_discard, total, [&, n, src, dst](size_t i) {
        value_t current = src[i];
        if (!std::isfinite(current)) {
            dst[i] = Null<price_t>();
            return;
        }
        int count = 0;
        size_t valid = 0;
        size_t start = i + 1 - n;
        for (size_t j = start; j <= i; ++j) {
            if (std::isfinite(src[j])) {
                valid++;
                if (src[j] <= current) {
                    count++;
                }
            }
        }
        dst[i] = static_cast<price_t>(count) / valid;
    });
}

size_t ITsRank::min_increment_start() const {
    return getParam<int>("n");
}

void ITsRank::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    for (size_t i = start_pos; i < total; ++i) {
        value_t current = src[i];
        if (!std::isfinite(current)) {
            dst[i] = Null<price_t>();
            continue;
        }
        int count = 0;
        size_t valid = 0;
        size_t start = i + 1 - n;
        for (size_t j = start; j <= i; ++j) {
            if (std::isfinite(src[j])) {
                valid++;
                if (src[j] <= current) {
                    count++;
                }
            }
        }
        dst[i] = static_cast<price_t>(count) / valid;
    }
}

void ITsRank::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (curPos < ind.discard() + step - 1) {
        return;
    }
    value_t current = ind[curPos];
    if (!std::isfinite(current)) {
        _set(Null<price_t>(), curPos);
        return;
    }
    size_t start = curPos + 1 - step;
    int count = 0;
    size_t valid = 0;
    for (size_t j = start; j <= curPos; ++j) {
        value_t v = ind[j];
        if (std::isfinite(v)) {
            valid++;
            if (v <= current) {
                count++;
            }
        }
    }
    _set(static_cast<price_t>(count) / valid, curPos);
}

Indicator HKU_API TS_RANK(int n) {
    IndicatorImpPtr p = make_shared<ITsRank>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API TS_RANK(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ITsRank>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */