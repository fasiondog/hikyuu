/*
 * BETWEEN.h
 *
 *  Created on: 2019年4月8日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BETWEEN_H_
#define INDICATOR_CRT_BETWEEN_H_

#include "CVAL.h"

namespace hku {

/**
 * 介于(介于两个数之间)
 * @details
 * <pre>
 * 用法：BETWEEN(A,B,C)表示A处于B和C之间时返回1，否则返回0
 * 例如：BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5))表示收盘价介于5日均线和10日均线之间
 * @ingroup Indicator
 * </pre>
 */
Indicator BETWEEN(const Indicator&, const Indicator&, const Indicator&);
Indicator BETWEEN(const Indicator&, const Indicator&, Indicator::value_type);
Indicator BETWEEN(const Indicator&, Indicator::value_type, const Indicator&);
Indicator BETWEEN(const Indicator&, Indicator::value_type, Indicator::value_type);
Indicator BETWEEN(Indicator::value_type, const Indicator&, const Indicator&);
Indicator BETWEEN(Indicator::value_type, const Indicator&, Indicator::value_type);
Indicator BETWEEN(Indicator::value_type, Indicator::value_type, const Indicator&);
Indicator BETWEEN(Indicator::value_type, Indicator::value_type, Indicator::value_type);

inline Indicator BETWEEN(const Indicator& a, const Indicator& b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(const Indicator& a, const Indicator& b, Indicator::value_type c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(const Indicator& a, Indicator::value_type b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(const Indicator& a, Indicator::value_type b, Indicator::value_type c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_type a, const Indicator& b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_type a, const Indicator& b, Indicator::value_type c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_type a, Indicator::value_type b, const Indicator& c) {
    Indicator result = IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
    result.name("BETWEEN");
    return result;
}

inline Indicator BETWEEN(Indicator::value_type a, Indicator::value_type b,
                         Indicator::value_type c) {
    Indicator result =
      CVAL((((b > c) && (a < b) && (a > c)) || ((b < c) && (a > b) && (a < c))) ? 1 : 0);
    result.name("BETWEEN");
    return result;
}

}  // namespace hku

#endif /* INDICATOR_CRT_BETWEEN_H_ */
