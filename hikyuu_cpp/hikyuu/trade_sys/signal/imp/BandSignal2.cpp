/*
 * BandSignal2.cpp
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */
#include "../../../indicator/crt/KDATA.h"
#include "BandSignal2.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::BandSignal2)
#endif

namespace hku {

BandSignal2::BandSignal2() : SignalBase("SG_Band") {}

BandSignal2::BandSignal2(const Indicator& ind, const Indicator& lower, const Indicator& upper)
: SignalBase("SG_Band"), m_ind(ind.clone()), m_lower(lower.clone()), m_upper(upper.clone()) {}

BandSignal2::~BandSignal2() {}

SignalPtr BandSignal2::_clone() {
    BandSignal2* p = new BandSignal2();
    p->m_upper = m_upper.clone();
    p->m_lower = m_lower.clone();
    p->m_ind = m_ind.clone();
    return SignalPtr(p);
}

void BandSignal2::_calculate(const KData& kdata) {
    Indicator ind = m_ind(kdata);
    Indicator upper = m_upper(kdata);
    Indicator lower = m_lower(kdata);
    HKU_ASSERT(ind.size() == upper.size() && ind.size() == lower.size());

    size_t discard = ind.discard();
    if (discard < upper.discard()) {
        discard = upper.discard();
    }
    if (discard < lower.discard()) {
        discard = lower.discard();
    }
    size_t total = ind.size();

    auto const* inddata = ind.data();
    auto const* upperdata = upper.data();
    auto const* lowerdata = lower.data();
    auto const* ks = kdata.data();
    for (size_t i = discard; i < total; ++i) {
        if (inddata[i] > upperdata[i]) {
            _addBuySignal(ks[i].datetime);
        } else if (inddata[i] < lowerdata[i]) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Band(const Indicator& sig, const Indicator& lower, const Indicator& upper) {
    return SignalPtr(new BandSignal2(sig, lower, upper));
}

}  // namespace hku