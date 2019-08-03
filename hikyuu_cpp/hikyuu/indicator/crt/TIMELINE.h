/*
 * TIMELINE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TIMELINE_H_
#define INDICATOR_CRT_TIMELINE_H_

#include "../Indicator.h"

namespace hku {

Indicator HKU_API TIMELINE();
Indicator HKU_API TIMELINE(const KData&);

Indicator TIMELINEVOL();
Indicator TIMELINEVOL(const KData&);

inline Indicator TIMELINEVOL() {
    Indicator ind = TIMELINE();
    ind.name("TIMELINEVOL");
    ind.setParam<string>("part", "vol");
    return ind;
}

inline Indicator TIMELINEVOL(const KData& k) {
    Indicator ind = TIMELINE(k);
    ind.name("TIMELINEVOL");
    ind.setParam<string>("part", "vol");
    return ind;
}


}

#endif /* INDICATOR_CRT_TIMELINE_H_ */
