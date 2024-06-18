/*
 * BandSignal.cpp
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */
#include "../../../indicator/crt/KDATA.h"
#include "BandSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::BandSignal)
#endif

namespace hku {

BandSignal::BandSignal() : SignalBase("SG_Band") {}

BandSignal::BandSignal(const Indicator& ind, price_t lower, price_t upper)
: SignalBase("SG_Band"), m_ind(ind.clone()), m_lower(lower), m_upper(upper) {
    HKU_CHECK(lower > upper, "BandSignal: lower track is greater than upper track");
}

BandSignal::~BandSignal() {}

SignalPtr BandSignal::_clone() {
    auto p = make_shared<BandSignal>();
    p->m_upper = m_upper;
    p->m_lower = m_lower;
    p->m_ind = m_ind.clone();
    return p;
}

void BandSignal::_calculate(const KData& kdata) {
    Indicator ind = m_ind(kdata);
    size_t discard = ind.discard();
    size_t total = ind.size();

    auto const* inddata = ind.data();
    auto const* ks = kdata.data();
    for (size_t i = discard; i < total; ++i) {
        if (inddata[i] > m_upper) {
            _addBuySignal(ks[i].datetime);
        } else if (inddata[i] < m_lower) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Band(const Indicator& sig, price_t lower, price_t upper) {
    return make_shared<BandSignal>(sig, lower, upper);
}

}  // namespace hku