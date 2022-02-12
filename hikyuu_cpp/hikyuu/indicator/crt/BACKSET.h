/*
 * BACKSET.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BACKSET_H_
#define INDICATOR_CRT_BACKSET_H_

#include "../Indicator.h"

namespace hku {

/**
 * 向前赋值将当前位置到若干周期前的数据设为1
 * @details
 * <pre>
 * 用法：BACKSET(X,N),X非0,则将当前位置到N周期前的数值设为1。
 * 例如：BACKSET(CLOSE>OPEN,2)若收阳则将该周期及前一周期数值设为1,否则为0
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BACKSET(int n = 2);
Indicator HKU_API BACKSET(const IndParam& n);

inline Indicator BACKSET(const Indicator& ind, int n = 2) {
    return BACKSET(n)(ind);
}

inline Indicator BACKSET(const Indicator& ind, const IndParam& n) {
    return BACKSET(n)(ind);
}

inline Indicator BACKSET(const Indicator& ind, const Indicator& n) {
    return BACKSET(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_BACKSET_H_ */
