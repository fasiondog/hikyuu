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

void ILog::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void ILog::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::log10(src[i]);
    }
}

Indicator HKU_API LOG() {
    return Indicator(make_shared<ILog>());
}

} /* namespace hku */
