/*
 * FixedHKTradeCost.h
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDHKTRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDHKTRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Hong Kong stock trade cost model.
 * @details
 * <pre>
 *   buy and sell are charged symmetrically (the stamp duty and the regulatory fees are
 *   two-sided in Hong Kong):
 *     fee = max(commission * price * number * unit, lowest_commission)
 *           + stamptax * price * number * unit
 *           + tradefee * price * number * unit
 *           + per_trade
 *   commission is the broker commission rate (double-sided, lifted to lowest_commission per
 *   order); stamptax is the stamp duty rate (0.1% since 2023-11-17, charged on both sides);
 *   tradefee aggregates the exchange trading fee and the SFC / AFIRD levies charged on both
 *   sides (0.00555% in total by default); per_trade is the fixed charge per order, e.g. the
 *   settlement fee booked by the broker. tradefee lands in CostRecord.transferfee and
 *   per_trade lands in CostRecord.others.
 * </pre>
 * @ingroup TradeCost
 */
class HKU_API FixedHKTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedHKTradeCost();
    FixedHKTradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                     price_t tradefee, price_t perTrade);
    virtual ~FixedHKTradeCost();

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

    virtual void _checkParam(const string& name) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;

private:
    /** buy and sell are charged symmetrically, so both entries share one calculation */
    CostRecord getCost(const Stock& stock, price_t price, double num) const;
};

}  // namespace hku

#endif /* TRADE_MANAGE_IMP_FIXEDHKTRADECOST_H_ */
