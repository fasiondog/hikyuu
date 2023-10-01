/*
 * BandSignal.cpp
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */
#include "../../../indicator/crt/KDATA.h"
#include "BandSignal.h"

namespace hku {

BandSignal::BandSignal() : SignalBase("SG_Band") {
    setParam<string>("kpart", "CLOSE");
}

BandSignal::BandSignal(const Indicator& ind, price_t lower, price_t upper, const string& kpart)
: SignalBase("SG_Band"), m_ind(ind), m_lower(lower), m_upper(upper) {
    setParam<string>("kpart", kpart);
    HKU_ERROR_IF(m_lower > m_upper, "BandSignal: lower track is greater than upper track");
}

BandSignal::~BandSignal() {}

SignalPtr BandSignal::_clone() {
    BandSignal* p = new BandSignal();
    p->m_upper = m_upper;
    p->m_lower = m_lower;
    p->m_ind = m_ind;
    return SignalPtr(p);
}

void BandSignal::_calculate() {
    string kpart = getParam<string>("kpart");
    Indicator kdata = KDATA_PART(m_kdata, kpart);

    Indicator ind = m_ind(kdata);
    size_t discard = ind.discard();
    size_t total = ind.size();

    for (size_t i = discard; i < total; ++i) {
        if (ind[i] > m_upper) {
            _addBuySignal(m_kdata[i].datetime);
        } else if (ind[i] < m_lower) {
            _addSellSignal(m_kdata[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Band(const Indicator& sig, price_t lower, price_t upper,
                          const string& kpart = "CLOSE") {
    return SignalPtr(new BandSignal(sig, lower, upper, kpart));
}

}  // namespace hku