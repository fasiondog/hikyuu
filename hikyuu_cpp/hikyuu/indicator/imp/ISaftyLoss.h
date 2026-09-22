/*
 * SaftyLoss.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef ISAFTYLOSS_H_
#define ISAFTYLOSS_H_

#include "../Indicator.h"

namespace hku {

/*
 * Alexander Elder's safe zone stop-loss
 * See "Come Into My Trading Room" (2007, Earthquake Press) by Alexander Elder, P202
 * Calculation description: within the lookback period (generally 10 to 20 days), add up the lengths
 *         of all the downward crossings and divide by the number of the downward crossings to get
 *         the average noise, and subtract (the previous day's average noise multiplied by a
 *         multiple) from today's low price to get the stop-loss line. To offset the fluctuation and
 *         guarantee that the stop-loss line moves upward, the highest value within N days
 * (generally 3 days) is taken based on the above result Note: the first (lookback period width +
 * the width for taking the highest value) points in the returned result are invalid Parameters: n1:
 * the lookback time window for calculating the average noise, 10 days by default n2: take the
 * highest value within n2 days for the preliminary stop-loss line, 3 by default p: the noise
 * coefficient, 2 by default
 */
class ISaftyLoss : public hku::IndicatorImp {
    INDICATOR_IMP(ISaftyLoss)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISaftyLoss();
    virtual ~ISaftyLoss() override;

    virtual void _checkParam(const string& name) const override;
    virtual void _dyn_calculate(const Indicator&) override;

private:
    void _dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2, double p);
};

} /* namespace hku */
#endif /* ISAFTYLOSS_H_ */
