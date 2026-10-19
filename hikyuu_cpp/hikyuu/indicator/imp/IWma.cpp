/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

#include <climits>
#include "IWma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IWma)
#endif

namespace hku {

IWma::IWma() : IndicatorImp("WMA", 1) {
    setParam<int>("n", 22);
}

IWma::~IWma() {}

void IWma::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_CHECK(getParam<int>("n") >= 1 && getParam<int>("n") < INT_MAX,
                  "WMA n must >= 1 and < INT_MAX!");
    }
}

void IWma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    if (n == 1) {
        memcpy(dst, src, total * sizeof(value_t));
        return;
    }

    // Non-finite entries are excluded from the weights; the output is the weighted mean of the
    // remaining entries (aligned with MA / STDEV / VAR), a fully missing window stays Null.
    value_t subsum = 0.0, sum = 0.0;
    size_t wsubsum = 0, wsum = 0;
    for (size_t i = ind.discard(), end = m_discard + 1, count = 1; i < end; i++, count++) {
        if (std::isfinite(src[i])) {
            subsum += src[i];
            sum += src[i] * count;
            wsubsum += 1;
            wsum += count;
        }
    }

    if (wsum > 0)
        dst[m_discard] = sum / wsum;

    size_t trailingIdx = ind.discard();
    for (size_t i = m_discard + 1; i < total; i++) {
        value_t tmp = src[i];
        value_t leaving = src[trailingIdx++];
        bool in_ok = std::isfinite(tmp);
        bool out_ok = std::isfinite(leaving);
        // shift existing weights by -1 (equivalent to subtracting the current subsum)
        sum -= subsum;
        wsum -= wsubsum;
        subsum += (in_ok ? tmp : 0.0) - (out_ok ? leaving : 0.0);
        wsubsum += (in_ok ? 1 : 0) - (out_ok ? 1 : 0);
        sum += in_ok ? tmp * n : 0.0;
        wsum += in_ok ? n : 0u;
        if (wsum > 0)
            dst[i] = sum / wsum;
    }
}

bool IWma::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IWma::min_increment_start() const {
    // The seed reads n elements before start_pos, so start_pos must be >= n.
    return getParam<int>("n");
}

void IWma::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    int n = getParam<int>("n");
    auto const* src = ind.data();
    auto* dst = this->data();

    value_t subsum = 0.0, sum = 0.0;
    size_t wsubsum = 0, wsum = 0;
    for (size_t i = start_pos - n, end = start_pos, count = 1; i < end; i++, count++) {
        if (std::isfinite(src[i])) {
            subsum += src[i];
            sum += src[i] * count;
            wsubsum += 1;
            wsum += count;
        }
    }

    size_t trailingIdx = start_pos - n;
    for (size_t i = start_pos; i < total; i++) {
        value_t tmp = src[i];
        value_t leaving = src[trailingIdx++];
        bool in_ok = std::isfinite(tmp);
        bool out_ok = std::isfinite(leaving);
        sum -= subsum;
        wsum -= wsubsum;
        subsum += (in_ok ? tmp : 0.0) - (out_ok ? leaving : 0.0);
        wsubsum += (in_ok ? 1 : 0) - (out_ok ? 1 : 0);
        sum += in_ok ? tmp * n : 0.0;
        wsum += in_ok ? n : 0u;
        if (wsum > 0)
            dst[i] = sum / wsum;
    }
}

void IWma::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (step < 1) {
        _set(Null<value_t>(), curPos);
        return;
    }

    if (step == 1) {
        _set(ind[curPos], curPos);
        return;
    }

    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < step + start) {
        _set(Null<value_t>(), curPos);
        return;
    }

    value_t sum = 0.0;
    size_t wsum = 0, n = 1;
    for (size_t i = start; i <= curPos; i++, n++) {
        if (std::isfinite(ind[i])) {
            sum += ind[i] * n;
            wsum += n;
        }
    }
    if (wsum == 0) {
        _set(Null<value_t>(), curPos);
        return;
    }
    _set(sum / wsum, curPos);
}

Indicator HKU_API WMA(int n) {
    IndicatorImpPtr p = make_shared<IWma>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API WMA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IWma>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
