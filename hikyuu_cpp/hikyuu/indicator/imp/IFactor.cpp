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
    // Factor 以“名字 + K线类型”作为唯一标识（见 Factor.h 文档），故意不比较公式内容。
    // 因此两个 FACTOR 节点仅在名字和 K线类型都相同时，才会被 CompiledFactorPlan
    // 当作同一节点做 CSE 合并。上游 FactorSet::add 已校验 K线类型并按名字去重
    //（后加入者覆盖先加入者），所以结构良好的 FactorSet 不会把“标识相同但公式不同”
    // 的两个因子同时喂进 compiled 路径。dynamic_cast 保证非 IFactor 节点不会判等。
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
