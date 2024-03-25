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

IDiff::IDiff() : IndicatorImp("DIFF", 1) {}

IDiff::~IDiff() {}

void IDiff::_calculate(const Indicator& data) {
    size_t total = data.size();

    m_discard = data.discard() + 1;
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = discard(); i < total; ++i) {
        dst[i] = src[i] - src[i - 1];
    }
}

Indicator HKU_API DIFF() {
    return Indicator(make_shared<IDiff>());
}

Indicator HKU_API DIFF(const Indicator& data) {
    return DIFF()(data);
}

} /* namespace hku */
