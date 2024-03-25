/*
 * ITimeLine.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */

#include "ITimeLine.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ITimeLine)
#endif

namespace hku {

ITimeLine::ITimeLine() : IndicatorImp("TIMELINE", 1) {
    setParam<string>("part", "price");
}

ITimeLine::~ITimeLine() {}

ITimeLine::ITimeLine(const KData& k) : IndicatorImp("TIMELINE", 1) {
    setParam<string>("part", "price");
    setParam<KData>("kdata", k);
    ITimeLine::_calculate(Indicator());
}

void ITimeLine::_checkParam(const string& name) const {
    if ("part" == name) {
        string part = getParam<string>("part");
        HKU_ASSERT(part == "price" || part == "vol");
    }
}

void ITimeLine::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    KQuery q = k.getQuery();
    Stock stk = k.getStock();

    TimeLineList time_line = stk.getTimeLineList(q);
    size_t total = time_line.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    auto* dst = this->data();

    m_discard = 0;
    if (getParam<string>("part") == "price") {
        for (size_t i = m_discard; i < total; i++) {
            dst[i] = time_line[i].price;
        }
    } else {
        for (size_t i = m_discard; i < total; i++) {
            dst[i] = time_line[i].vol;
        }
    }
}

Indicator HKU_API TIMELINE() {
    return make_shared<ITimeLine>()->calculate();
}

Indicator HKU_API TIMELINE(const KData& k) {
    return Indicator(make_shared<ITimeLine>(k));
}

} /* namespace hku */
