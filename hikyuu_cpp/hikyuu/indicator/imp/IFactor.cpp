/*
 * IFactor.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#include "IFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IFactor)
#endif

namespace hku {

IFactor::IFactor() : IndicatorImp("FACTOR", 1) {
    m_need_context = true;
    m_need_self_alike_compare = true;
}

IFactor::IFactor(const Factor& factor) : IndicatorImp("FACTOR", 1), m_factor(factor) {
    m_need_context = true;
    m_need_self_alike_compare = true;
}

IFactor::~IFactor() {}

string IFactor::formula() const {
    return m_factor.formula().formula();
};

IndicatorImpPtr IFactor::_clone() {
    return make_shared<IFactor>(m_factor);
}

bool IFactor::selfAlike(const IndicatorImp& other) const noexcept {
    // Factor uses the "name + K-line type" as the unique identifier (see the Factor.h
    // documentation) and deliberately does not compare the formula content. Therefore two FACTOR
    // nodes are treated as the same node by CompiledFactorPlan for the CSE merge only when both the
    // name and the K-line type are the same. The upstream FactorSet::add has already verified the
    // K-line type and removed the duplicates by name (the later one overwrites the earlier one), so
    // a well-formed FactorSet would not feed two factors with the same identifier but different
    // formulas into the compiled path at the same time. dynamic_cast guarantees that a non-IFactor
    // node is not judged equal.
    const auto* other_ctx = dynamic_cast<const IFactor*>(&other);
    HKU_IF_RETURN(other_ctx == nullptr, false);
    return m_factor.name() == other_ctx->m_factor.name() &&
           m_factor.ktype() == other_ctx->m_factor.ktype();
}

void IFactor::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    auto value = m_factor.getValue(k);
    value.setContext(k);
    m_discard = value.discard();
    value.getImp()->swap(this);
}

Indicator HKU_API FACTOR(const Factor& factor) {
    return Indicator(make_shared<IFactor>(factor));
}

} /* namespace hku */
