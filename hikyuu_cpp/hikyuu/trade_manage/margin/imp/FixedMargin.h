/*
 * FixedMargin.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXED_MARGIN_H_
#define TRADE_MANAGE_IMP_FIXED_MARGIN_H_

#include "../MarginBase.h"

namespace hku {

/**
 * Fixed margin model, either by a ratio of the nominal value or by a fixed amount per lot
 * @details
 * <pre>
 * rate mode:          margin = rate(side) * price * num * unit
 * per-lot mode:       margin = margin_per_lot * ceil(num / minTradeNumber)
 * maintain margin:    maintain_rate / rate(side) * open margin, when maintain_rate > 0
 * </pre>
 * @param rate opening margin ratio, (0, 1], 0.6 by default
 * @param rate_short short opening ratio, 0 means following rate
 * @param maintain_rate maintain margin ratio, 0 means the same as the opening margin
 * @param margin_per_lot margin of one lot, 0 means using the rate mode
 * @ingroup Margin
 */
class HKU_API FixedMargin : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedMargin();
    virtual ~FixedMargin();

    virtual price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num, MarginSide side) const override;

    virtual price_t getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num, MarginSide side) const override;

    virtual double getMarginRate(const Datetime& datetime, const Stock& stock,
                                 MarginSide side) const override;

    virtual MarginPtr _clone() override;

protected:
    virtual void _checkParam(const string& name) const override;

private:
    /** The ratio of the given side, rate_short falls back to rate when it is 0 */
    double rateOf(MarginSide side) const;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_FIXED_MARGIN_H_ */
