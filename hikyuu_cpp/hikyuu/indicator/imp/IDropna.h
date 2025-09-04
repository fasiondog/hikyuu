/*
 * IDropna.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IDROPNA_H_
#define INDICATOR_IMP_IDROPNA_H_

#include "../Indicator.h"

namespace hku {

class IDropna : public IndicatorImp {
    INDICATOR_IMP(IDropna)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDropna();
    virtual ~IDropna();

    virtual bool needSelfAlikeCompare() const noexcept override {
        return true;
    }

    // 禁止子节点合并, 上级默认已为 false
    // virtual bool selfAlike(const IndicatorImp& other) const noexcept override {
    //     return false;
    // }
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IDROPNA_H_ */
