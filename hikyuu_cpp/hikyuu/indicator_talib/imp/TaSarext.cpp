/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaSarext.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaSarext)
#endif

namespace hku {

TaSarext::TaSarext() : IndicatorImp("TA_SAREXT", 1) {
    setParam<double>("startvalue", 0.0);
    setParam<double>("offsetonreverse", 0.0);
    setParam<double>("accelerationinitlong", 0.02);
    setParam<double>("accelerationlong", 0.02);
    setParam<double>("accelerationmaxlong", 0.2);
    setParam<double>("accelerationinitshort", 0.02);
    setParam<double>("accelerationshort", 0.02);
    setParam<double>("accelerationmaxshort", 0.2);
}

TaSarext::TaSarext(double startvalue, double offsetonreverse, double accelerationinitlong,
                   double accelerationlong, double accelerationmaxlong,
                   double accelerationinitshort, double accelerationshort,
                   double accelerationmaxshort)
: IndicatorImp("TA_SAREXT", 1) {
    setParam<double>("startvalue", startvalue);
    setParam<double>("offsetonreverse", offsetonreverse);
    setParam<double>("accelerationinitlong", accelerationinitlong);
    setParam<double>("accelerationlong", accelerationlong);
    setParam<double>("accelerationmaxlong", accelerationmaxlong);
    setParam<double>("accelerationinitshort", accelerationinitshort);
    setParam<double>("accelerationshort", accelerationshort);
    setParam<double>("accelerationmaxshort", accelerationmaxshort);
}
TaSarext::TaSarext(const KData& k, double startvalue, double offsetonreverse,
                   double accelerationinitlong, double accelerationlong, double accelerationmaxlong,
                   double accelerationinitshort, double accelerationshort,
                   double accelerationmaxshort)
: IndicatorImp("TA_SAREXT", 1) {
    setParam<KData>("kdata", k);
    setParam<double>("startvalue", startvalue);
    setParam<double>("offsetonreverse", offsetonreverse);
    setParam<double>("accelerationinitlong", accelerationinitlong);
    setParam<double>("accelerationlong", accelerationlong);
    setParam<double>("accelerationmaxlong", accelerationmaxlong);
    setParam<double>("accelerationinitshort", accelerationinitshort);
    setParam<double>("accelerationshort", accelerationshort);
    setParam<double>("accelerationmaxshort", accelerationmaxshort);
    TaSarext::_calculate(Indicator());
}

void TaSarext::_checkParam(const string& name) const {
    if (name == "startvalue") {
        HKU_ASSERT(!std::isnan(getParam<double>(name)));
    } else if (name == "offsetonreverse" || name == "accelerationlong" ||
               name == "accelerationshort" || name == "accelerationmaxshort" ||
               name == "accelerationinitlong" || name == "accelerationmaxlong" ||
               name == "accelerationinitshort") {
        double p = getParam<double>(name);
        HKU_CHECK(p >= 0, "{} must >= 0!", name);
    }
}

void TaSarext::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    double startvalue = getParam<double>("startvalue");
    double offsetonreverse = getParam<double>("offsetonreverse");
    double accelerationinitlong = getParam<double>("accelerationinitlong");
    double accelerationlong = getParam<double>("accelerationlong");
    double accelerationmaxlong = getParam<double>("accelerationmaxlong");
    double accelerationinitshort = getParam<double>("accelerationinitshort");
    double accelerationshort = getParam<double>("accelerationshort");
    double accelerationmaxshort = getParam<double>("accelerationmaxshort");
    int back = TA_SAREXT_Lookback(startvalue, offsetonreverse, accelerationinitlong,
                                  accelerationlong, accelerationmaxlong, accelerationinitshort,
                                  accelerationshort, accelerationmaxshort);
    if (back < 0 || back >= total) {
        m_discard = total;
        return;
    }

    const KRecord* kptr = k.data();
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);
    double* high = buf.get();
    double* low = high + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
    }

    auto* dst = this->data();
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_SAREXT(m_discard, total - 1, high, low, startvalue, offsetonreverse, accelerationinitlong,
              accelerationlong, accelerationmaxlong, accelerationinitshort, accelerationshort,
              accelerationmaxshort, &outBegIdx, &outNbElement, dst + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_SAREXT(double startvalue, double offsetonreverse, double accelerationinitlong,
                            double accelerationlong, double accelerationmaxlong,
                            double accelerationinitshort, double accelerationshort,
                            double accelerationmaxshort) {
    return Indicator(make_shared<TaSarext>(
      startvalue, offsetonreverse, accelerationinitlong, accelerationlong, accelerationmaxlong,
      accelerationinitshort, accelerationshort, accelerationmaxshort));
}

Indicator HKU_API TA_SAREXT(const KData& k, double startvalue, double offsetonreverse,
                            double accelerationinitlong, double accelerationlong,
                            double accelerationmaxlong, double accelerationinitshort,
                            double accelerationshort, double accelerationmaxshort) {
    return Indicator(make_shared<TaSarext>(
      k, startvalue, offsetonreverse, accelerationinitlong, accelerationlong, accelerationmaxlong,
      accelerationinitshort, accelerationshort, accelerationmaxshort));
}

} /* namespace hku */
