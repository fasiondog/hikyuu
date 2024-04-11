/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */

#include "IResult.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IResult)
#endif

namespace hku {

IResult::IResult() : IndicatorImp("RESULT", 1) {
    setParam<int>("result_ix", 0);
}

IResult::IResult(int result_ix) : IndicatorImp("RESULT", 1) {
    setParam<int>("result_ix", result_ix);
    checkParam("result_ix");
}

void IResult::_checkParam(const string& name) const {
    if ("result_ix" == name) {
        int result_ix = getParam<int>("result_ix");
        HKU_ASSERT(result_ix >= 0 && result_ix < MAX_RESULT_NUM);
    }
}

void IResult::_calculate(const Indicator& ind) {
    int result_ix = getParam<int>("result_ix");
    HKU_CHECK(result_ix < ind.getResultNumber(),
              "The input indicator has only {} results, but result_ix({}) is out_of range!",
              ind.getResultNumber(), result_ix);
    m_discard = ind.discard();
    HKU_IF_RETURN(m_discard >= ind.size(), void());

    const auto* src = ind.data(result_ix);
    auto* dst = this->data();
    memcpy(dst + m_discard, src + m_discard, sizeof(value_t) * (ind.size() - m_discard));
}

Indicator HKU_API RESULT(int result_ix) {
    return Indicator(make_shared<IResult>(result_ix));
}

}  // namespace hku
