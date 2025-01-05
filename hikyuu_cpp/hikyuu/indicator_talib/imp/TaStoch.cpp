/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaStoch.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaStoch)
#endif

namespace hku {

TaStoch::TaStoch() : IndicatorImp("TA_STOCH", 2) {
    setParam<int>("fastk_n", 5);
    setParam<int>("slowk_n", 3);
    setParam<int>("slowk_matype", 0);
    setParam<int>("slowd_n", 3);
    setParam<int>("slowd_matype", 0);
}

TaStoch::TaStoch(const KData& k, int fastk_n, int slowk_n, int slowk_matype, int slowd_n,
                 int slowd_matype)
: IndicatorImp("TA_STOCH", 2) {
    setParam<KData>("kdata", k);
    setParam<int>("fastk_n", fastk_n);
    setParam<int>("slowk_n", slowk_n);
    setParam<int>("slowk_matype", slowk_matype);
    setParam<int>("slowd_n", slowd_n);
    setParam<int>("slowd_matype", slowd_matype);
    TaStoch::_calculate(Indicator());
}

void TaStoch::_checkParam(const string& name) const {
    if (name == "fastk_n" || name == "slowk_n" || name == "slowd_n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
    } else if (name == "slowk_matype" || name == "slowd_matype") {
        int matype = getParam<int>(name);
        HKU_CHECK(matype >= 0 && matype <= 8, "{} must in [0, 8]", name);
    }
}

void TaStoch::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 2);

    int fastk_n = getParam<int>("fastk_n");
    int slowk_n = getParam<int>("slowk_n");
    TA_MAType slowk_matype = (TA_MAType)getParam<int>("slowk_matype");
    int slowd_n = getParam<int>("slowd_n");
    TA_MAType slowd_matype = (TA_MAType)getParam<int>("slowd_matype");
    int back = TA_STOCH_Lookback(fastk_n, slowk_n, slowk_matype, slowd_n, slowd_matype);
    if (back < 0 || back >= total) {
        m_discard = total;
        return;
    }

    const KRecord* kptr = k.data();
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);
    double* high = buf.get();
    double* low = high + total;
    double* close = low + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
        close[i] = kptr[i].closePrice;
    }

    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_STOCH(m_discard, total - 1, high, low, close, fastk_n, slowk_n, slowk_matype, slowd_n,
             slowd_matype, &outBegIdx, &outNbElement, dst0 + m_discard, dst1 + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_STOCH(int fastk_n, int slowk_n, int slowk_matype, int slowd_n,
                           int slowd_matype) {
    auto p = make_shared<TaStoch>();
    p->setParam<int>("fastk_n", fastk_n);
    p->setParam<int>("slowk_n", slowk_n);
    p->setParam<int>("slowk_matype", slowk_matype);
    p->setParam<int>("slowd_n", slowd_n);
    p->setParam<int>("slowd_matype", slowd_matype);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_STOCH(const KData& k, int fastk_n, int slowk_n, int slowk_matype, int slowd_n,
                           int slowd_matype) {
    return Indicator(
      make_shared<TaStoch>(k, fastk_n, slowk_n, slowk_matype, slowd_n, slowd_matype));
}

} /* namespace hku */
