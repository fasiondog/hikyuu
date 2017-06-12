/*
 * CrossSignal.cpp
 *
 *  Created on: 2015年2月20日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
#include "CrossSignal.h"

namespace hku {

CrossSignal::CrossSignal() : SignalBase("SG_Cross") {
    setParam<string>("kpart", "CLOSE");
}

CrossSignal::CrossSignal(const Operand& fast,
        const Operand& slow,
        const string& kpart)
: SignalBase("SG_Cross"), m_fast(fast), m_slow(slow) {
    setParam<string>("kpart", kpart);
}


CrossSignal::~CrossSignal() {

}

SignalPtr CrossSignal::_clone() {
    CrossSignal* p = new CrossSignal();
    p->m_fast = m_fast;
    p->m_slow = m_slow;
    return SignalPtr(p);
}

void CrossSignal::_calculate() {
    string kpart = getParam<string>("kpart");
    Indicator kdata = KDATA_PART(m_kdata, kpart);
    Indicator fast = m_fast(kdata);
    Indicator slow = m_slow(kdata);
    if (fast.size() != slow.size()) {
        HKU_ERROR("fast.size() != slow.size() [CrossSignal::_calculate]");
        return;
    }

    size_t discard = fast.discard() > slow.discard()
            ? fast.discard() : slow.discard();

    size_t total = fast.size();
    for (size_t i = discard + 1; i < total; ++i) {
        if (fast[i-1] < slow[i-1] && fast[i] > slow[i]) {
            _addBuySignal(m_kdata[i].datetime);
        } else if (fast[i-1] > slow[i-1] && fast[i] < slow[i]) {
            _addSellSignal(m_kdata[i].datetime);
        }
    }
}


SignalPtr HKU_API SG_Cross(const Operand& fast,
        const Operand& slow, const string& kpart) {
    return SignalPtr(new CrossSignal(fast, slow, kpart));
}

} /* namespace hku */
