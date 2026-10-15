/*
 * BandSignal2.cpp
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */
#include <cmath>
#include "../../../indicator/crt/ALIGN.h"
#include "../../../indicator/crt/KDATA.h"
#include "BandSignal2.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::BandSignal2)
#endif

namespace hku {

BandSignal2::BandSignal2() : SignalBase("SG_Band2") {}

BandSignal2::BandSignal2(const Indicator& ind, const Indicator& lower, const Indicator& upper)
: SignalBase("SG_Band2"), m_ind(ind.clone()), m_lower(lower.clone()), m_upper(upper.clone()) {}

BandSignal2::~BandSignal2() {}

SignalPtr BandSignal2::_clone() {
    auto p = make_shared<BandSignal2>();
    p->m_upper = m_upper.clone();
    p->m_lower = m_lower.clone();
    p->m_ind = m_ind.clone();
    return p;
}

void BandSignal2::_calculate(const KData& kdata) {
    Indicator ind = m_ind(kdata);
    if (ind.size() != kdata.size()) {
        ind = ALIGN(ind, kdata);
    }
    Indicator upper = m_upper(kdata);
    if (upper.size() != kdata.size()) {
        upper = ALIGN(upper, kdata);
    }
    Indicator lower = m_lower(kdata);
    if (lower.size() != kdata.size()) {
        lower = ALIGN(lower, kdata);
    }
    HKU_WARN_IF_RETURN(
      ind.size() != kdata.size() || upper.size() != kdata.size() || lower.size() != kdata.size(),
      void(),
      "Failed to align! ind.size()({}), upper.size()({}), lower.size()({}), "
      "kdata.size()({})!",
      ind.size(), upper.size(), lower.size(), kdata.size());

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
        if (std::isfinite(inddata[i]) && std::isfinite(upperdata[i]) && inddata[i] > upperdata[i]) {
            _addBuySignal(ks[i].datetime);
        } else if (std::isfinite(inddata[i]) && std::isfinite(lowerdata[i]) &&
                   inddata[i] < lowerdata[i]) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Band(const Indicator& sig, const Indicator& lower, const Indicator& upper) {
    return make_shared<BandSignal2>(sig, lower, upper);
}

}  // namespace hku