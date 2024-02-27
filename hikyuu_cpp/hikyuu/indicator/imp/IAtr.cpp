/*
 * IAtr.cpp
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#include "../crt/ATR.h"
#include "../crt/SLICE.h"
#include "IAtr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAtr)
#endif

namespace hku {

IAtr::IAtr() : IndicatorImp("ATR", 1) {
    setParam<int>("n", 14);
}

IAtr::~IAtr() {}

bool IAtr::check() {
    return getParam<int>("n") >= 1;
}

void IAtr::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    size_t startPos = discard();

    auto const* src = indicator.data();
    auto* dst = this->data();
    dst[startPos] = src[startPos];
    value_t multiplier = 2.0 / (n + 1);
    for (size_t i = startPos + 1; i < total; ++i) {
        dst[i] = (src[i] - dst[i - 1]) * multiplier + dst[i - 1];
    }
}

void IAtr::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 1, void());
    Indicator slice = SLICE(ind, 0, curPos + 1);
    Indicator atr = ATR(slice, step);
    if (atr.size() > 0) {
        _set(atr[atr.size() - 1], curPos);
    }
}

Indicator HKU_API ATR(int n) {
    IndicatorImpPtr p = make_shared<IAtr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API ATR(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IAtr>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
