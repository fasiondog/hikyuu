/*
 * CrossGoldSignal.cpp
 *
 *  Created on: 2017年6月13日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
#include "CrossGoldSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::CrossGoldSignal)
#endif

namespace hku {

CrossGoldSignal::CrossGoldSignal() : SignalBase("SG_CrossGold") {}

CrossGoldSignal::CrossGoldSignal(const Indicator& fast, const Indicator& slow)
: SignalBase("SG_CrossGold"), m_fast(fast), m_slow(slow) {}

CrossGoldSignal::~CrossGoldSignal() {}

SignalPtr CrossGoldSignal::_clone() {
    auto p = make_shared<CrossGoldSignal>();
    p->m_fast = m_fast.clone();
    p->m_slow = m_slow.clone();
    return p;
}

void CrossGoldSignal::_calculate(const KData& kdata) {
    Indicator fast = m_fast(kdata);
    Indicator slow = m_slow(kdata);
    HKU_ERROR_IF_RETURN(fast.size() != slow.size(), void(), "fast.size() != slow.size()");

    size_t discard = fast.discard() > slow.discard() ? fast.discard() : slow.discard();
    size_t total = fast.size();
    auto const* fastdata = fast.data();
    auto const* slowdata = slow.data();
    auto const* ks = kdata.data();
    for (size_t i = discard + 1; i < total; ++i) {
        if (fastdata[i - 1] < slowdata[i - 1] && fastdata[i] > slowdata[i] &&
            fastdata[i - 1] < fastdata[i] && slowdata[i - 1] < slowdata[i]) {
            _addBuySignal(ks[i].datetime);
        } else if (fastdata[i - 1] > slowdata[i - 1] && fastdata[i] < slowdata[i] &&
                   fastdata[i - 1] > fastdata[i] && slowdata[i - 1] > slowdata[i]) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_CrossGold(const Indicator& fast, const Indicator& slow) {
    return make_shared<CrossGoldSignal>(fast, slow);
}

} /* namespace hku */
