/*
 * IExp.cpp
 *
 *  Created on: 2019年4月3日
 *      Author: fasiondog
 */

#include "IExp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IExp)
#endif

namespace hku {

IExp::IExp() : IndicatorImp("EXP", 1) {}

IExp::~IExp() {}

bool IExp::check() {
    return true;
}

void IExp::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::exp(src[i]);
    }
}

Indicator HKU_API EXP() {
    return Indicator(make_shared<IExp>());
}

} /* namespace hku */
