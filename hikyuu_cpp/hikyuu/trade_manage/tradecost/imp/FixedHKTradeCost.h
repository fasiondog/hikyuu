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
 *   buy and sell are symmetric: stamp duty and regulatory fees are two-sided in Hong Kong
 *     fee = max(commission * price * number * unit, lowest_commission)
 *           + ceil(stamptax * price * number * unit)   // rounded up to a whole HK dollar
 *           + tradefee * price * number * unit
 *           + clamp(settlefee * price * number * unit, settlefee_min, settlefee_max)
 *           + per_trade
 *   commission: broker rate, two-sided, lifted to lowest_commission per order
 *   stamptax: 0.1% since 2023-11-17, two-sided, rounded up to a whole HK dollar per order (HKEX)
 *   tradefee: exchange 0.00565% + SFC 0.0027% + AFIRD 0.00015%, two-sided (0.0085% default)
 *   settlefee: CCASS ratio (0.002% in market, 0 here), bounded by settlefee_min/max (0 = off)
 *   per_trade: fixed charge per order
 *   tradefee and settlefee -> CostRecord.transferfee; per_trade -> CostRecord.others
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
    /** Buy and sell are symmetric, so both share one calculation */
    CostRecord getCost(const Stock& stock, price_t price, double num) const;
};

}  // namespace hku

#endif /* TRADE_MANAGE_IMP_FIXEDHKTRADECOST_H_ */
