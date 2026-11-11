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
 *           + ceil(stamptax * price * number * unit)   // rounded up to a whole HK dollar
 *           + tradefee * price * number * unit
 *           + clamp(settlefee * price * number * unit, settlefee_min, settlefee_max)
 *           + per_trade
 *   commission is the broker commission rate (double-sided, lifted to lowest_commission per
 *   order); stamptax is the stamp duty rate (0.1% since 2023-11-17, charged on both sides and
 *   rounded up to a whole HK dollar per order, as the HKEX does); tradefee aggregates the
 *   exchange trading fee (0.00565%) and the SFC (0.0027%) / AFIRD (0.00015%) levies charged on
 *   both sides (0.0085% in total by default); settlefee is the CCASS settlement fee ratio
 *   (0.002% by default in the market, 0 here) optionally bounded by settlefee_min / settlefee_max
 *   (0 means the bound is not applied); per_trade is the fixed charge per order. tradefee and
 *   settlefee land in CostRecord.transferfee, per_trade lands in CostRecord.others.
 * </pre>
 * @ingroup TradeCost
 */
class HKU_API FixedHKTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedHKTradeCost();
    FixedHKTradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                     price_t tradefee, price_t perTrade, price_t settlefee = 0.0,
                     price_t settleFeeMin = 0.0, price_t settleFeeMax = 0.0);
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
