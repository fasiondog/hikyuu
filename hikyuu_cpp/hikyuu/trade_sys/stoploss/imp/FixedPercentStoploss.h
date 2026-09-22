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
 * Fixed percentage stop-loss strategy, i.e. the stop-loss is triggered when the price is lower than
 * the buy price by a certain percentage
 */
class FixedPercentStoploss : public StoplossBase {
    STOPLOSS_IMP(FixedPercentStoploss, "FixedPercentSL")
    STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedPercentStoploss();
    virtual ~FixedPercentStoploss();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* FIXEDPERCENTSTOPLOSS_H_ */
