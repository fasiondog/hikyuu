/*
 * ILiuTongPang.cpp
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#include "ILiuTongPan.h"
#include "StockWeightFill.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILiuTongPan)
#endif

namespace hku {

ILiuTongPan::ILiuTongPan() : IndicatorImp("LIUTONGPAN", 1) {
    m_need_context = true;
}

ILiuTongPan::~ILiuTongPan() {}

void ILiuTongPan::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    auto* dst = this->data();
    fill_by_weight_series(
      k, 0, [](const StockWeight& w) { return w.freeCount(); },
      [dst](size_t pos, const KRecord&, price_t pre_free_count) { dst[pos] = pre_free_count; });

    updateDiscard(true);
}

Indicator HKU_API LIUTONGPAN() {
    return make_shared<ILiuTongPan>()->calculate();
}

Indicator HKU_API LIUTONGPAN(const KData& k) {
    auto p = make_shared<ILiuTongPan>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
