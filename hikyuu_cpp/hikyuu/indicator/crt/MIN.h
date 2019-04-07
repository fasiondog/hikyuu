/*
 * MIN.h
 *
 *  Created on: 2019年4月8日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_MIN_H_
#define INDICATOR_CRT_MIN_H_

#include "CVAL.h"

namespace hku {

Indicator MIN(const Indicator&, const Indicator&);
Indicator MIN(const Indicator&, price_t val);
Indicator MIN(price_t val, const Indicator& ind);

inline Indicator MIN(const Indicator& ind1, const Indicator& ind2) {
    Indicator result = IF(ind1 < ind2, ind1, ind2);
    result.name("MIN");
    return result;
}

inline Indicator MIN(const Indicator& ind, price_t val) {
    Indicator result = IF(ind < val, ind, val);
    result.name("MIN");
    return result;
}

inline Indicator MIN(price_t val, const Indicator& ind) {
    Indicator result = IF(val < ind, val, ind);
    result.name("MIN");
    return result;
}

}


#endif /* INDICATOR_CRT_MIN_H_ */
