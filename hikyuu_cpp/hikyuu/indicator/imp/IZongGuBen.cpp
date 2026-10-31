/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

#include "IZongGuBen.h"
#include "StockWeightFill.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IZongGuBen)
#endif

namespace hku {

IZongGuBen::IZongGuBen() : IndicatorImp("ZONGGUBEN", 1) {
    m_need_context = true;
}

IZongGuBen::~IZongGuBen() {}

void IZongGuBen::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);
    _increment_calculate(data, 0);
    updateDiscard(true);
}

void IZongGuBen::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0 || start_pos >= total, void());

    auto* dst = this->data();
    fill_by_weight_series(
      k, start_pos, [](const StockWeight& w) { return w.totalCount(); },
      [dst](size_t pos, const KRecord&, price_t pre_total_count) { dst[pos] = pre_total_count; });
}

Indicator HKU_API ZONGGUBEN() {
    return make_shared<IZongGuBen>()->calculate();
}

Indicator HKU_API ZONGGUBEN(const KData& k) {
    auto p = make_shared<IZongGuBen>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
