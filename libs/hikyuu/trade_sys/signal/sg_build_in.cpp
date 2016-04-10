/*
 * sg_build_in.cpp
 *
 *  Created on: 2016年4月10日
 *      Author: Administrator
 */

#include "../../indicator/crt/KDATA.h"
#include "sg_build_in.h"
#include "imp/CrossSignal.h"
#include "imp/SingleSignal.h"
#include "imp/FlexSignal.h"


namespace hku {

SignalPtr HKU_API SG_Cross(const Indicator& fast, const Indicator& slow,
        const string& kpart) {
    CrossSignal *p = new CrossSignal(fast, slow);
    //p->setParam<string>("kpart", kpart);
    return SignalPtr(p);
}

SignalPtr HKU_API SG_Cross(const Operand& fast, const Operand& slow) {
    CrossSignal *p = new CrossSignal(fast, slow);
    //p->setParam<string>("kpart", kpart);
    return SignalPtr(p);
}

SignalPtr HKU_API SG_Single(const Operand& ind,
        int filter_n, double filter_p, const string& kpart) {
    SingleSignal *p = new SingleSignal(ind);
    p->setParam<int>("filter_n", filter_n);
    p->setParam<double>("filter_p", filter_p);
    p->setParam<string>("kpart", kpart);
    return SignalPtr(p);
}


SignalPtr HKU_API SG_Flex(const Indicator& ind, double p, const string& kpart) {
    FlexSignal *ptr = new FlexSignal(ind);
    ptr->setParam<double>("p", p);
    ptr->setParam<string>("kpart", kpart);
    return SignalPtr(ptr);
}

} /* namespace hku */

