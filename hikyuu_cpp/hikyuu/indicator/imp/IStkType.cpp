/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

#include "hikyuu/StockTypeInfo.h"
#include "IStkType.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IStkType)
#endif

namespace hku {

IStkType::IStkType() : IndicatorImp("STKTYPE", 1) {
    m_need_context = true;
}

IStkType::~IStkType() {}

void IStkType::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!",
                getParam<string>("kpart"));

    size_t total = getContext().size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    m_discard = 0;
    _increment_calculate(ind, 0);
}

void IStkType::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& kdata = getContext();
    size_t total = kdata.size();
    const Stock& stock = kdata.getStock();
    value_t stktype = static_cast<value_t>(stock.type());
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; i++) {
        dst[i] = stktype;
    }
}

Indicator HKU_API STKTYPE() {
    return make_shared<IStkType>()->calculate();
}

Indicator HKU_API STKTYPE(const KData& k) {
    auto p = make_shared<IStkType>();
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku