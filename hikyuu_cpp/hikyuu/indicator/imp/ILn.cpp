/*
 * ILn.cpp
 *
 *  Created on: 2019年4月11日
 *      Author: fasiondog
 */

#include "ILn.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILn)
#endif

namespace hku {

ILn::ILn() : IndicatorImp("LN", 1) {}

ILn::~ILn() {}

void ILn::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void ILn::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::log(src[i]);
    }
}

Indicator HKU_API LN() {
    return Indicator(make_shared<ILn>());
}

} /* namespace hku */
