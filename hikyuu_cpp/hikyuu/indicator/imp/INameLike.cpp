/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-10
 *      Author: fasiondog
 */

#include "hikyuu/Stock.h"
#include "INameLike.h"
#include "WildcardMatch.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::INameLike)
#endif

namespace hku {

INameLike::INameLike() : IndicatorImp("NAMELIKE", 1) {
    m_need_context = true;
    setParam<string>("pattern", "");
}

INameLike::~INameLike() {}

void INameLike::_calculate(const Indicator& data) {
    HKU_IF_RETURN(!isLeaf() && !data.empty(), void());

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    string pattern = getParam<string>("pattern");
    string name = k.getStock().name();
    value_t match = wildcardMatch(name, pattern) ? 1.0 : 0.0;

    auto* dst = this->data();
    for (size_t i = 0; i < total; ++i) {
        dst[i] = match;
    }
}

void INameLike::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& k = getContext();
    size_t total = k.size();

    string pattern = getParam<string>("pattern");
    string name = k.getStock().name();
    value_t match = wildcardMatch(name, pattern) ? 1.0 : 0.0;

    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
        dst[i] = match;
    }
}

Indicator HKU_API NAMELIKE(const string& pattern) {
    auto p = make_shared<INameLike>();
    p->setParam<string>("pattern", pattern);
    return Indicator(p);
}

Indicator HKU_API NAMELIKE(const KData& k, const string& pattern) {
    auto p = make_shared<INameLike>();
    p->setParam<string>("pattern", pattern);
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
