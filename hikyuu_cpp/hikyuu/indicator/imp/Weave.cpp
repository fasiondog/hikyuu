/*
 * Weave.cpp
 *
 *  Created on: 2017年4月19日
 *      Author: Administrator
 */

#include "../Indicator.h"

namespace hku {

Indicator HKU_API WEAVE(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        HKU_ERROR("ind1 or ind2 is Null Indicator! [WEAVE]");
        return Indicator();
    }
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::TWO, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

} /* namespace hku */
