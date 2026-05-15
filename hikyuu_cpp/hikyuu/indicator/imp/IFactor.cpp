/*
 * IFactor.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "IFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IFactor)
#endif

namespace hku {

IFactor::IFactor() : IndicatorImp("FACTOR", 1) {
    m_need_context = true;
}

IFactor::IFactor(const Factor& factor) : IndicatorImp("FACTOR", 1), m_factor(factor) {
    m_need_context = true;
}

IFactor::~IFactor() {}

string IFactor::formula() const {
    return m_factor.formula().formula();
};

IndicatorImpPtr IFactor::_clone() {
    return make_shared<IFactor>(m_factor);
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
