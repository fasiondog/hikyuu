/*
 * FixedTradeCost.h
 *
 *  Created on: 2026-11-07
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDTRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDTRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Generic fixed-rate trade cost algorithm: the futures / FX / crypto model, applicable to any
 * market without a dedicated cost model.
 * @details
 * <pre>
 *   buy:  fee = max(commission * price * number * unit, lowest_commission) + per_lot * number
 *               + others
 *   sell: the same plus stamptax * price * number * unit
 *   commission and stamptax are rounded to the price precision of the security; per_lot and
 *   others are exact and both land in CostRecord.others (they do not depend on the turnover)
 *   unit is the contract multiplier of the security: 1 for stocks, the contract size for
 *   futures, so one formula covers both without a type branch
 * </pre>
 * @ingroup TradeCost
 */
class HKU_API FixedTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedTradeCost();
    FixedTradeCost(price_t commission, price_t lowestCommission, price_t stamptax, price_t perLot,
                   price_t others);
    virtual ~FixedTradeCost();

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
};

}  // namespace hku

#endif /* TRADE_MANAGE_IMP_FIXEDTRADECOST_H_ */
