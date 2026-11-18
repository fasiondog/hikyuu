/*
 * FixedETFTradeCost.h
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDETFTRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDETFTRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

class FixedETFTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedETFTradeCost();
    virtual ~FixedETFTradeCost();

    virtual void _checkParam(const string& name) const override;

    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    virtual TradeCostPtr _clone() override;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_FIXEDETFTRADECOST_H_ */