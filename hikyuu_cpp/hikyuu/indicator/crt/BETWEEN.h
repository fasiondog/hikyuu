/*
 * BETWEEN.h
 *
 *  Created on: 2019-4-8
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BETWEEN_H_
#define INDICATOR_CRT_BETWEEN_H_

#include "CVAL.h"

namespace hku {

/**
 * Between (between two numbers)
 * @details
 * <pre>
 * Usage: BETWEEN(A,B,C) returns 1 when A is between B and C, otherwise it returns 0
 * For example: BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5)) means the close price is between the 5-day
 * moving average and the 10-day moving average
 * @ingroup Indicator
 * </pre>
 */
Indicator BETWEEN(const Indicator&, const Indicator&, const Indicator&);
Indicator BETWEEN(const Indicator&, const Indicator&, Indicator::value_t);
Indicator BETWEEN(const Indicator&, Indicator::value_t, const Indicator&);
Indicator BETWEEN(const Indicator&, Indicator::value_t, Indicator::value_t);
Indicator BETWEEN(Indicator::value_t, const Indicator&, const Indicator&);
Indicator BETWEEN(Indicator::value_t, const Indicator&, Indicator::value_t);
Indicator BETWEEN(Indicator::value_t, Indicator::value_t, const Indicator&);
Indicator BETWEEN(Indicator::value_t, Indicator::value_t, Indicator::value_t);

inline Indicator BETWEEN(const Indicator& a, const Indicator& b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(const Indicator& a, const Indicator& b, Indicator::value_t c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(const Indicator& a, Indicator::value_t b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(const Indicator& a, Indicator::value_t b, Indicator::value_t c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_t a, const Indicator& b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_t a, const Indicator& b, Indicator::value_t c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_t a, Indicator::value_t b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_t a, Indicator::value_t b, Indicator::value_t c) {
    Indicator result =
      CVAL((((b > c) && (a < b) && (a > c)) || ((b < c) && (a > b) && (a < c))) ? 1 : 0);
    result.name("BETWEEN");
    return result;
}

}  // namespace hku

#endif /* INDICATOR_CRT_BETWEEN_H_ */
