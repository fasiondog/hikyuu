/*
 * FixedATradeCost.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDATRADECOST_H_
#define FIXEDATRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share; it calculates the cost of every buy
 * or sell
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: both the commission and the transfer fee have a minimum value; the current commission
 *   ratio is 1.8 per mille (5 yuan minimum), and the stamp duty is 1 per mille
 *         The transfer fee of the Shanghai Stock Exchange is 1 per mille of the traded quantity,
 * and it is counted as one yuan when it is less than 1 yuan
 * </pre>
 */
class HKU_API FixedATradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    /**
     * Default constructor, it also sets the default parameter values
     * @details
     * <pre>
     * Commission ratio, 1.8 per mille by default, i.e. 0.0018
     * Minimum commission value, 5 yuan by default
     * Stamp duty, 1 per mille by default, i.e. 0.001
     * Transfer fee, 1 per mille per share by default, i.e. 0.001
     * Minimum transfer fee, 1 yuan by default
     * </pre>
     */
    FixedATradeCost();

    /**
     * @param commission commission ratio
     * @param lowestCommission minimum commission value
     * @param stamptax stamp duty
     * @param transferfee transfer fee
     * @param lowestTransferfee minimum transfer fee
     */
    FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                    price_t transferfee, price_t lowestTransferfee);
    virtual ~FixedATradeCost();

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
#endif /* FIXEDATRADECOST_H_ */
