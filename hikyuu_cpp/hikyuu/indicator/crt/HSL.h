/*
 * HSL.h
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_HSL_H_
#define INDICATOR_CRT_HSL_H_

#include "KDATA.h"
#include "LIUTONGPAN.h"
#include "HSL.h"

namespace hku {

/**
 * 获取换手率，等于 VOL(k) / CAPITAL(k)
 * @param k 关联的K线数据
 * @ingroup Indicator
 */
Indicator HSL(const KData& k);
Indicator HSL();

inline Indicator HSL() {
    Indicator hsl = VOL() / LIUTONGPAN();
    hsl.name("HSL");
    return hsl;
}

inline Indicator HSL(const KData& k) {
    Indicator hsl = HSL();
    hsl.setContext(k);
    return hsl;
}

}  // namespace hku
#endif /* INDICATOR_CRT_HSL_H_ */
