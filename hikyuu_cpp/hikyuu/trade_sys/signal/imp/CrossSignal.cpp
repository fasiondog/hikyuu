/*
 * CrossSignal.cpp
 *
 *  Created on: 2015年2月20日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
#include "CrossSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::CrossSignal)
#endif

namespace hku {

CrossSignal::CrossSignal() : SignalBase("SG_Cross") {}

CrossSignal::CrossSignal(const Indicator& fast, const Indicator& slow)
: SignalBase("SG_Cross"), m_fast(fast), m_slow(slow) {}

CrossSignal::~CrossSignal() {}

SignalPtr CrossSignal::_clone() {
    CrossSignal* p = new CrossSignal();
    p->m_fast = m_fast.clone();
    p->m_slow = m_slow.clone();
    return SignalPtr(p);
}

void CrossSignal::_calculate() {
    Indicator fast = m_fast(m_kdata);
    Indicator slow = m_slow(m_kdata);
    HKU_ERROR_IF_RETURN(fast.size() != slow.size(), void(), "fast.size() != slow.size()");

    size_t discard = fast.discard() > slow.discard() ? fast.discard() : slow.discard();
    size_t total = fast.size();
    for (size_t i = discard + 1; i < total; ++i) {
        if (fast[i - 1] < slow[i - 1] && fast[i] > slow[i]) {
            _addBuySignal(m_kdata[i].datetime);
        } else if (fast[i - 1] > slow[i - 1] && fast[i] < slow[i]) {
            _addSellSignal(m_kdata[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Cross(const Indicator& fast, const Indicator& slow) {
    return SignalPtr(new CrossSignal(fast, slow));
}

} /* namespace hku */
