/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-22
 *      Author: fasiondog
 */

#include "IHsl.h"
#include "StockWeightFill.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IHsl)
#endif

namespace hku {

IHsl::IHsl() : IndicatorImp("HSL", 1) {
    m_need_context = true;
}

IHsl::~IHsl() {}

void IHsl::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    // transCount is in lots on the daily line and above but in shares below it; freeCount is in
    // units of 10 thousand shares. The factor normalizes both to the turnover ratio (x 100 = %).
    bool day_or_above =
      KQuery::getKTypeInSeconds(k.getQuery().kType()) >= KQuery::getKTypeInSeconds(KQuery::DAY);
    value_t hsl_factor = day_or_above ? 0.01 : 0.0001;

    _readyBuffer(total, 1);
    auto* dst = this->data();
    fill_by_weight_series(
      k, 0, [](const StockWeight& w) { return w.freeCount(); },
      [dst, hsl_factor](size_t pos, const KRecord& krecord, price_t pre_free_count) {
          dst[pos] = krecord.transCount / pre_free_count * hsl_factor;
      });

    updateDiscard(true);
}

Indicator HKU_API HSL() {
    return make_shared<IHsl>()->calculate();
}

Indicator HKU_API HSL(const KData& k) {
    auto p = make_shared<IHsl>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
