/*
 * Ama.h
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#pragma once
#ifndef IAMA_H_
#define IAMA_H_

#include "../Indicator.h"

namespace hku {

/*
 * Perry J. Kaufman adaptive moving average, see "Smarter Trading" (2006, Guangdong Economy
 * Publishing House)
 * Parameters: n: the period window for calculating the average, it must be an integer greater than
 * 2 fast_n: the period of the fast trend, generally 2, it does not need to be changed slow_n: the N
 * value of the corresponding slow EMA line, Kaufman generally sets it to 30; the indicator
 * converges when it exceeds about 60 and there is not much influence
 */
class IAma : public IndicatorImp {
    INDICATOR_IMP(IAma)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAma();
    virtual ~IAma() override;

    virtual void _checkParam(const string& name) const override;
    virtual void _dyn_calculate(const Indicator&) override;

    virtual size_t min_increment_start() const override;

private:
    void _dyn_one_circle(const Indicator& ind, size_t curPos, int n, int fast_n, int slow_n);
};

} /* namespace hku */
#endif /* IAMA_H_ */
