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

HKU_API SignalPtr operator&(const SignalPtr& sg1, const SignalPtr& sg2);
HKU_API SignalPtr operator|(const SignalPtr& sg1, const SignalPtr& sg2);

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

inline SignalPtr SG_Add(const vector<SignalPtr> sg_list, bool alternate) {
    HKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
    SignalPtr tmp = SG_Add(sg_list[0], sg_list[1], alternate);
    for (size_t i = 2; i < sg_list.size(); ++i) {
        tmp = SG_Add(tmp, sg_list[i], alternate);
    }
    return tmp;
}

inline SignalPtr SG_Sub(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 - sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_Sub(const vector<SignalPtr> sg_list, bool alternate) {
    HKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
    SignalPtr tmp = SG_Sub(sg_list[0], sg_list[1], alternate);
    for (size_t i = 2; i < sg_list.size(); ++i) {
        tmp = SG_Sub(tmp, sg_list[i], alternate);
    }
    return tmp;
}

inline SignalPtr SG_Mul(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 * sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_Mul(const vector<SignalPtr> sg_list, bool alternate) {
    HKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
    SignalPtr tmp = SG_Mul(sg_list[0], sg_list[1], alternate);
    for (size_t i = 2; i < sg_list.size(); ++i) {
        tmp = SG_Mul(tmp, sg_list[i], alternate);
    }
    return tmp;
}

inline SignalPtr SG_Div(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 / sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_Div(const vector<SignalPtr> sg_list, bool alternate) {
    HKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
    SignalPtr tmp = SG_Div(sg_list[0], sg_list[1], alternate);
    for (size_t i = 2; i < sg_list.size(); ++i) {
        tmp = SG_Div(tmp, sg_list[i], alternate);
    }
    return tmp;
}

inline SignalPtr SG_And(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 & sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_And(const vector<SignalPtr> sg_list, bool alternate) {
    HKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
    SignalPtr tmp = SG_And(sg_list[0], sg_list[1], alternate);
    for (size_t i = 2; i < sg_list.size(); ++i) {
        tmp = SG_And(tmp, sg_list[i], alternate);
    }
    return tmp;
}

inline SignalPtr SG_Or(const SignalPtr& sg1, const SignalPtr& sg2, bool alternate) {
    auto sg = sg1 | sg2;
    sg->setParam<bool>("alternate", alternate);
    return sg;
}

inline SignalPtr SG_Or(const vector<SignalPtr> sg_list, bool alternate) {
    HKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
    SignalPtr tmp = SG_Or(sg_list[0], sg_list[1], alternate);
    for (size_t i = 2; i < sg_list.size(); ++i) {
        tmp = SG_Or(tmp, sg_list[i], alternate);
    }
    return tmp;
}

}  // namespace hku