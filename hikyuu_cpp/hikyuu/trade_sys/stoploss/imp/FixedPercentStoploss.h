/*
 * FixedPercentStoploss.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDPERCENTSTOPLOSS_H_
#define FIXEDPERCENTSTOPLOSS_H_

#include "../StoplossBase.h"

namespace hku {

/**
 * 固定百分比止损策略，即当价格低于买入价格的某一百分比时止损
 */
class FixedPercentStoploss : public StoplossBase {
    STOPLOSS_IMP(FixedPercentStoploss, "FixedPercentSL")
    STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedPercentStoploss();
    virtual ~FixedPercentStoploss();
    virtual void _checkParam(const string& name) const;
};

} /* namespace hku */
#endif /* FIXEDPERCENTSTOPLOSS_H_ */
