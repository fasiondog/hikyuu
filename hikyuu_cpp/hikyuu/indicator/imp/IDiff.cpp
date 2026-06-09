/*
 * IDiff.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "IDiff.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDiff)
#endif

namespace hku {

IDiff::IDiff() : IndicatorImp("DIFF", 1) {
    setParam<int>("n", 1);
}

IDiff::~IDiff() {}

void IDiff::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") > 0);
    }
}

void IDiff::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n;
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IDiff::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = src[i] - src[i - n];
    }
}

Indicator HKU_API DIFF(int n) {
    IndicatorImpPtr p = make_shared<IDiff>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API DIFF(const Indicator& data, int n) {
    return DIFF(n)(data);
}

} /* namespace hku */