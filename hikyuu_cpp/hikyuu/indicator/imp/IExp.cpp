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

void IExp::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IExp::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::exp(src[i]);
    }
}

Indicator HKU_API EXP() {
    return Indicator(make_shared<IExp>());
}

} /* namespace hku */
