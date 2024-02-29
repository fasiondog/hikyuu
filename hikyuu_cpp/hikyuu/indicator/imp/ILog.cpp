/*
 * ILog.cpp
 *
 *  Created on: 2019年4月11日
 *      Author: fasiondog
 */

#include "ILog.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILog)
#endif

namespace hku {

ILog::ILog() : IndicatorImp("LOG", 1) {}

ILog::~ILog() {}

bool ILog::check() {
    return true;
}

void ILog::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    value_t null_price = Null<value_t>();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = (src[i] <= 0.0) ? null_price : std::log10(data[i]);
    }
}

Indicator HKU_API LOG() {
    return Indicator(make_shared<ILog>());
}

} /* namespace hku */
