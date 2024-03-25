/*
 * IFilter.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "IFilter.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IFilter)
#endif

namespace hku {

IFilter::IFilter() : IndicatorImp("FILTER", 1) {
    setParam<int>("n", 5);
}

IFilter::~IFilter() {}

void IFilter::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IFilter::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (0 == n) {
        for (size_t i = m_discard; i < total; i++) {
            dst[i] = src[i] != 0.0 ? 1.0 : 0.0;
        }
        return;
    }

    size_t i = m_discard;
    while (i < total) {
        if (src[i] == 0.0) {
            dst[i] = 0.0;
            i++;
        } else {
            dst[i] = 1.0;
            size_t end = i + n + 1;
            if (end > total) {
                end = total;
            }
            for (size_t j = i + 1; j < end; j++) {
                dst[j] = 0.0;
            }
            i = end;
        }
    }
}

void IFilter::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    price_t val = get(curPos);
    HKU_IF_RETURN(!std::isnan(val) && val == 0.0, void());
    if (ind[curPos] == 0.0) {
        _set(0.0, curPos);
    } else {
        _set(1.0, curPos);
        size_t end = curPos + step + 1;
        if (end > ind.size()) {
            end = ind.size();
        }
        for (size_t i = curPos + 1; i < end; i++) {
            _set(0.0, i);
        }
    }
}

Indicator HKU_API FILTER(int n) {
    IndicatorImpPtr p = make_shared<IFilter>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API FILTER(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IFilter>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
