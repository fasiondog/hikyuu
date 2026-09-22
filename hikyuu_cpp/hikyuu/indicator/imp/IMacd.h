/*
 * IMacd.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_MACD_H_
#define INDICATOR_IMP_MACD_H_

#include "../Indicator.h"

namespace hku {

/*
 * MACD moving average convergence / divergence

 * Parameters: n1: short-term EMA time window

 *         n2: long-term EMA time window

 *         n3: EMA smoothing time window of (short-term EMA - long-term EMA)

 * Returns: 1) MACD BAR: MACD histogram, i.e. MACD fast line - MACD slow line

 *      2) DIFF: fast line, i.e. (short-term EMA - long-term EMA)

 *      3) DEA: slow line, i.e. the n3-period EMA smoothing of the fast line

 */
class IMacd : public IndicatorImp {
    INDICATOR_IMP(IMacd)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IMacd();
    virtual ~IMacd() override;

    virtual void _checkParam(const string& name) const override;
    virtual void _dyn_calculate(const Indicator&) override;
    virtual size_t min_increment_start() const override;

private:
    void _dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2, int n3);
};

} /* namespace hku */
#endif /* INDICATOR_IMP_MACD_H_ */
