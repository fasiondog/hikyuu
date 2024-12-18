/*
 * TaCmo.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "ta-lib/ta_func.h"
#include "TaCmo.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaCmo)
#endif

namespace hku {

TaCmo::TaCmo() : IndicatorImp("T_CMO", 1) {
    setParam<int>("n", 14);
}

TaCmo::~TaCmo() {}

void TaCmo::_checkParam(const string &name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    }
}

void TaCmo::_calculate(const Indicator &data) {
    int n = getParam<int>("n");
    m_discard = data.discard() + TA_CMO_Lookback(n);
    size_t total = data.size();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();

    int outBegIdx;
    int outNbElement;
    TA_CMO(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement, dst + m_discard);

    // TA_LIB_API TA_RetCode TA_CMO( int    startIdx,
    //                               int    endIdx,
    //                                          const double inReal[],
    //                                          int           optInTimePeriod,
    //                                          int          *outBegIdx, int *outNBElement,
    //                                          double        outReal[] );
}

Indicator HKU_API TA_CMO(int n) {
    auto p = make_shared<TaCmo>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
