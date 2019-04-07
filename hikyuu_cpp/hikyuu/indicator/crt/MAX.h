/*
 * MAX.h
 *
 *  Created on: 2019年4月8日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_MAX_H_
#define INDICATOR_CRT_MAX_H_

#include "CVAL.h"

namespace hku {

Indicator MAX(const Indicator&, const Indicator&);
Indicator MAX(const Indicator&, price_t val);
Indicator MAX(price_t val, const Indicator& ind);

inline Indicator MAX(const Indicator& ind1, const Indicator& ind2) {
    Indicator result = IF(ind1 > ind2, ind1, ind2);
    result.name("MAX");
    return result;
}

inline Indicator MAX(const Indicator& ind, price_t val) {
    Indicator result = IF(ind > val, ind, val);
    result.name("MAX");
    return result;
}

inline Indicator MAX(price_t val, const Indicator& ind) {
    Indicator result = IF(val > ind, val, ind);
    result.name("MAX");
    return result;
}

}


#endif /* INDICATOR_CRT_MAX_H_ */
