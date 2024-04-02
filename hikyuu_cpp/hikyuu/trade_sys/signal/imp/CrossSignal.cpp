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

void CrossSignal::_calculate(const KData& kdata) {
    Indicator fast = m_fast(kdata);
    Indicator slow = m_slow(kdata);
    HKU_ERROR_IF_RETURN(fast.size() != slow.size(), void(), "fast.size() != slow.size()");

    size_t discard = fast.discard() > slow.discard() ? fast.discard() : slow.discard();
    size_t total = fast.size();
    auto const* fastdata = fast.data();
    auto const* slowdata = slow.data();
    auto const* ks = kdata.data();
    for (size_t i = discard + 1; i < total; ++i) {
        if (fastdata[i - 1] < slowdata[i - 1] && fastdata[i] > slowdata[i]) {
            _addBuySignal(ks[i].datetime);
        } else if (fastdata[i - 1] > slowdata[i - 1] && fastdata[i] < slowdata[i]) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Cross(const Indicator& fast, const Indicator& slow) {
    return SignalPtr(new CrossSignal(fast, slow));
}

} /* namespace hku */
