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

void ILog::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::log10(src[i]);
    }
}

Indicator HKU_API LOG() {
    return Indicator(make_shared<ILog>());
}

} /* namespace hku */
