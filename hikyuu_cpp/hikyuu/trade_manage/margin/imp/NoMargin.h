/*
 * NoMargin.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_NO_MARGIN_H_
#define TRADE_MANAGE_IMP_NO_MARGIN_H_

#include "../MarginBase.h"

namespace hku {

/** Plain cash model: an opening consumes the full nominal and no position occupies margin */
class HKU_API NoMargin : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NoMargin();
    virtual ~NoMargin();

    virtual bool isMarginTrading(const Stock& stock) const override;

    virtual price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num, MarginSide side) const override;

    virtual MarginPtr _clone() override;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_NO_MARGIN_H_ */
