/*
 * ZeroTradeCost.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef ZEROTRADECOST_H_
#define ZEROTRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

class HKU_API ZeroTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ZeroTradeCost();
    virtual ~ZeroTradeCost();

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
#endif /* ZEROTRADECOST_H_ */
