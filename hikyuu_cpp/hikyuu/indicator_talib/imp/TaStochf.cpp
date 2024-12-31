/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaStochf.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaStochf)
#endif

namespace hku {

TaStochf::TaStochf() : IndicatorImp("TA_STOCHF", 2) {
    setParam<int>("fastk_n", 5);
    setParam<int>("fastd_n", 3);
    setParam<int>("fastd_matype", 0);
}

TaStochf::TaStochf(const KData& k, int fastk_n, int fastd_n, int fastd_matype)
: IndicatorImp("TA_STOCHF", 2) {
    setParam<KData>("kdata", k);
    setParam<int>("fastk_n", fastk_n);
    setParam<int>("fastd_n", fastd_n);
    setParam<int>("fastd_matype", fastd_matype);
    TaStochf::_calculate(Indicator());
}

void TaStochf::_checkParam(const string& name) const {
    if (name == "fastk_n" || name == "fastd_n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
    } else if (name == "fastd_matype") {
        int fastd_matype = getParam<int>("fastd_matype");
        HKU_ASSERT(fastd_matype >= 0 && fastd_matype <= 8);
    }
}

void TaStochf::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 2);

    int fastk_n = getParam<int>("fastk_n");
    int fastd_n = getParam<int>("fastd_n");
    TA_MAType fastd_matype = (TA_MAType)getParam<int>("fastd_matype");
    int back = TA_STOCHF_Lookback(fastk_n, fastd_n, fastd_matype);
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
    TA_STOCHF(m_discard, total - 1, high, low, close, fastk_n, fastd_n, fastd_matype, &outBegIdx,
              &outNbElement, dst0 + m_discard, dst1 + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_STOCHF(int fastk_n, int fastd_n, int fastd_matype) {
    auto p = make_shared<TaStochf>();
    p->setParam<int>("fastk_n", fastk_n);
    p->setParam<int>("fastd_n", fastd_n);
    p->setParam<int>("fastd_matype", fastd_matype);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_STOCHF(const KData& k, int fastk_n, int fastd_n, int fastd_matype) {
    return Indicator(make_shared<TaStochf>(k, fastk_n, fastd_n, fastd_matype));
}

} /* namespace hku */
