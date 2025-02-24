/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "../SignalBase.h"

namespace hku {

HKU_API SignalPtr operator+(const SignalPtr& sg1, const SignalPtr& sg2);
HKU_API SignalPtr operator-(const SignalPtr& sg1, const SignalPtr& sg2);
HKU_API SignalPtr operator*(const SignalPtr& sg1, const SignalPtr& sg2);
HKU_API SignalPtr operator/(const SignalPtr& sg1, const SignalPtr& sg2);

HKU_API SignalPtr operator+(const SignalPtr& sg, double value);
HKU_API SignalPtr operator-(const SignalPtr& sg, double value);
HKU_API SignalPtr operator*(const SignalPtr& sg, double value);
HKU_API SignalPtr operator/(const SignalPtr& sg, double value);

HKU_API SignalPtr operator+(double value, const SignalPtr& sg);
HKU_API SignalPtr operator-(double value, const SignalPtr& sg);
HKU_API SignalPtr operator*(double value, const SignalPtr& sg);
HKU_API SignalPtr operator/(double value, const SignalPtr& sg);

inline SignalPtr operator&(const SignalPtr& sg1, const SignalPtr& sg2) {
    return sg1 * sg2;
}

inline SignalPtr operator|(const SignalPtr& sg1, const SignalPtr& sg2) {
    return sg1 + sg2;
}

//------------------------------------------------------------------
// 由于 SG 的 alternate 默认为 True, 在使用如  "sg1 + sg2 + sg3"
// 的形式时，容易忽略 sg1 + sg2 的 alternate 属性，故添加 SG_Add 等函数
// 建议使用: SG_Add(sg1, sg2, False) + sg3 来避免 alternate 的问题
//------------------------------------------------------------------
inline SignalPtr SG_Add(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 + sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_Sub(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 - sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_Mul(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 * sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_Div(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 / sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

}  // namespace hku