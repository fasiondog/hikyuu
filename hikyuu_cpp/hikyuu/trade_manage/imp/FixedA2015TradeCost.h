/*
 * AShareTradeCost.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDA2015TRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDA2015TRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

class FixedA2015TradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedA2015TradeCost();
    virtual ~FixedA2015TradeCost();

    virtual void _checkParam(const string& name) const override;

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_FIXEDA2015TRADECOST_H_ */
