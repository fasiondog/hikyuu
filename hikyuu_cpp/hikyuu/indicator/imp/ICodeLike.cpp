/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#include "hikyuu/Stock.h"
#include "ICodeLike.h"
#include "WildcardMatch.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICodeLike)
#endif

namespace hku {

ICodeLike::ICodeLike() : IndicatorImp("CODELIKE", 1) {
    m_need_context = true;
    setParam<string>("pattern", "");
}

ICodeLike::~ICodeLike() {}

void ICodeLike::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    _readyBuffer(total, 1);
    HKU_IF_RETURN(total == 0, void());

    string pattern = getParam<string>("pattern");
    string code = k.getStock().code();
    value_t match = wildcardMatch(code, pattern) ? 1.0 : 0.0;

    auto* dst = this->data();
    for (size_t i = 0; i < total; ++i) {
        dst[i] = match;
    }
}

void ICodeLike::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& k = getContext();
    size_t total = k.size();

    string pattern = getParam<string>("pattern");
    string code = k.getStock().code();
    value_t match = wildcardMatch(code, pattern) ? 1.0 : 0.0;

    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
        dst[i] = match;
    }
}

Indicator HKU_API CODELIKE(const string& pattern) {
    auto p = make_shared<ICodeLike>();
    p->setParam<string>("pattern", pattern);
    return Indicator(p);
}

Indicator HKU_API CODELIKE(const KData& k, const string& pattern) {
    auto p = make_shared<ICodeLike>();
    p->setParam<string>("pattern", pattern);
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
