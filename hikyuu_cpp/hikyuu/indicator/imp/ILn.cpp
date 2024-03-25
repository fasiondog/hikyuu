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

    auto const* src = data.data();
    auto* dst = this->data();
    value_t null_value = Null<value_t>();
    for (size_t i = m_discard; i < total; ++i) {
        if (data[i] <= 0.0) {
            dst[i] = null_value;
        } else {
            dst[i] = std::log(src[i]);
        }
    }
}

Indicator HKU_API LN() {
    return Indicator(make_shared<ILn>());
}

} /* namespace hku */
