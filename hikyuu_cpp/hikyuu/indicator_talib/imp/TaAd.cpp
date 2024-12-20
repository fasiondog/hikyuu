/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaAd.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaAd)
#endif

namespace hku {

TaAd::TaAd() : IndicatorImp("TA_AD", 1) {}

TaAd::TaAd(const KData& k) : IndicatorImp("TA_AD", 1) {
    setParam<KData>("kdata", k);
    TaAd::_calculate(Indicator());
}

void TaAd::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

    const KRecord* kptr = k.data();
    std::unique_ptr<double[]> high = std::make_unique<double[]>(total);
    std::unique_ptr<double[]> low = std::make_unique<double[]>(total);
    std::unique_ptr<double[]> close = std::make_unique<double[]>(total);
    std::unique_ptr<double[]> vol = std::make_unique<double[]>(total);
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
        close[i] = kptr[i].closePrice;
        vol[i] = kptr[i].transCount;
    }

    auto* dst = this->data();

    int outBegIdx;
    int outNbElement;
    TA_AD(0, total - 1, high.get(), low.get(), close.get(), vol.get(), &outBegIdx, &outNbElement,
          dst);
}

Indicator HKU_API TA_AD() {
    return make_shared<TaAd>()->calculate();
}

Indicator HKU_API TA_AD(const KData& k) {
    return Indicator(make_shared<TaAd>(k));
}

} /* namespace hku */
