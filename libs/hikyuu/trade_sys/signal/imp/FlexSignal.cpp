/*
 * FlexSignal.cpp
 *
 *  Created on: 2015年3月21日
 *      Author: Administrator
 */

#include "../../../indicator/crt/EMA.h"
#include "../../../indicator/crt/KDATA.h"
#include "FlexSignal.h"

namespace hku {

FlexSignal::FlexSignal() : SignalBase("FLEX") {
    setParam<string>("kpart", "CLOSE");
    setParam<double>("p", 2.0);
}

FlexSignal::FlexSignal(const Indicator& ind)
: SignalBase("FLEX"), m_ind(ind) {
    setParam<string>("kpart", "CLOSE");
    setParam<double>("p", 2.0);
}

FlexSignal::~FlexSignal() {

}

SignalPtr FlexSignal::_clone() {
    FlexSignal* p = new FlexSignal();
    p->m_ind = m_ind;
    return SignalPtr(p);
}

void FlexSignal::_calculate() {
    string kpart(getParam<string>("kpart"));
    double p = getParam<double>("p");

    //如果m_ind有参数n
    int slow_n;
    try {
        int fast_n = m_ind.getParam<int>("n");
        slow_n = fast_n * (int)p;
    } catch(...) {
        slow_n = int(p);
    }

    Indicator fast = m_ind(KDATA_PART(m_kdata, kpart));
    Indicator slow = EMA(fast, slow_n);

    size_t discard = slow.discard();
    size_t total = fast.size();
    for (size_t i = discard + 1; i < total; ++i) {
        if (fast[i-1] < slow[i-1] && fast[i] >= slow[i]) {
            _addBuySignal(m_kdata[i].datetime);
        } else if (fast[i-1] > slow[i-1] && fast[i] <= slow[i]) {
            _addSellSignal(m_kdata[i].datetime);
        }
    }
}

SignalPtr HKU_API Flex_SG(const Indicator& ind, double p, const string& kpart) {
    FlexSignal *ptr = new FlexSignal(ind);
    ptr->setParam<double>("p", p);
    ptr->setParam<string>("kpart", kpart);
    return SignalPtr(ptr);
}

} /* namespace hku */
