/*
 * ITsIc.cpp
 *
 *  Created on: 2026-11-2
 *      Author: fasiondog
 */

#include <algorithm>
#include <climits>
#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/REF.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "../utils.h"
#include "ITsIc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ITsIc)
#endif

namespace hku {

ITsIc::ITsIc() : IndicatorImp("TS_IC", 1) {
    setParam<int>("n", 30);    // The rolling correlation window
    setParam<int>("ic_n", 1);  // The return span paired with the factor
    setParam<bool>("use_spearman", true);
    setParam<bool>("strict", false);
}

ITsIc::~ITsIc() {}

void ITsIc::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2 && getParam<int>("n") < INT_MAX);
    } else if ("ic_n" == name) {
        HKU_ASSERT(getParam<int>("ic_n") >= 1 && getParam<int>("ic_n") < INT_MAX);
    }
}

size_t ITsIc::min_increment_start() const {
    return getParam<int>("n") + getParam<int>("ic_n");
}

void ITsIc::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    int ic_n = getParam<int>("ic_n");

    // def(ind) is evaluated eagerly without a context, so fall back to the input factor's KData
    KData k = getContext();
    if (k.size() != total) {
        KData in_k = ind.getContext();
        if (in_k.size() != total) {
            m_discard = total;
            return;
        }
        k = in_k;
    }
    Indicator f = REF(ind, ic_n);
    Indicator r = ROCP(CLOSE(), ic_n)(k);

    const auto* fd = f.data();
    const auto* rd = r.data();
    auto* dst = this->data();

    bool use_spearman = getParam<bool>("use_spearman");
    // Optimistic prefix only: a non-finite pair (Pearson) or a zero-variance window leaves the
    // first bars Null, so updateDiscard() below rescans for the real first finite value
    m_discard = ind.discard() + (size_t)ic_n + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    global_parallel_for_index_void(m_discard, total, [&, n, fd, rd, dst, use_spearman](size_t i) {
        const auto* pf = fd + i + 1 - n;
        const auto* pr = rd + i + 1 - n;
        dst[i] = use_spearman ? detail::cross_section_spearman(pf, pr, n)
                              : detail::cross_section_pearson(pf, pr, n);
    });

    if (getParam<bool>("strict")) {
        // Index by the factor date: the value at i-ic_n owns the return completing at i, and the
        // trailing ic_n slots have no realized forward return yet
        for (size_t i = m_discard; i < total; i++) {
            dst[i - ic_n] = dst[i];
        }
        if (total > (size_t)ic_n) {
            for (size_t i = total - ic_n; i < total; i++) {
                dst[i] = Null<price_t>();
            }
        }
        m_discard = 0;  // rescan the head, the shift moved the first value earlier
    }

    updateDiscard();
}

void ITsIc::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    int ic_n = getParam<int>("ic_n");

    KData k = getContext();
    auto* dst = this->data();
    if (k.size() != total) {
        // As in the full path, then clear everything: the framework has already shifted the old
        // values in place, so a partial buffer would read as a valid result
        KData in_k = ind.getContext();
        if (in_k.size() != total) {
            std::fill(dst, dst + total, Null<price_t>());
            return;
        }
        k = in_k;
    }
    Indicator f = REF(ind, ic_n);
    Indicator r = ROCP(CLOSE(), ic_n)(k);

    const auto* fd = f.data();
    const auto* rd = r.data();

    bool use_spearman = getParam<bool>("use_spearman");
    global_parallel_for_index_void(start_pos, total, [&, n, fd, rd, dst, use_spearman](size_t i) {
        const auto* pf = fd + i + 1 - n;
        const auto* pr = rd + i + 1 - n;
        dst[i] = use_spearman ? detail::cross_section_spearman(pf, pr, n)
                              : detail::cross_section_pearson(pf, pr, n);
    });

    if (getParam<bool>("strict")) {
        // Mirror the full path's shifted layout; the framework guarantees start_pos >= n + ic_n,
        // so i - ic_n never underflows
        for (size_t i = start_pos; i < total; i++) {
            dst[i - ic_n] = dst[i];
        }
        if (total > (size_t)ic_n) {
            for (size_t i = total - ic_n; i < total; i++) {
                dst[i] = Null<price_t>();
            }
        }
    }
}

Indicator HKU_API TS_IC(int n, int ic_n, bool spearman, bool strict) {
    auto p = make_shared<ITsIc>();
    p->setParam<int>("n", n);
    p->setParam<int>("ic_n", ic_n);
    p->setParam<bool>("use_spearman", spearman);
    p->setParam<bool>("strict", strict);
    return Indicator(p);
}

} /* namespace hku */
