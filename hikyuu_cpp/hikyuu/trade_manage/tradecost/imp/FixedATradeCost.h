/*
 * FixedATradeCost.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDATRADECOST_H_
#define FIXEDATRADECOST_H_

#include "FixedATradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the Shanghai, Shenzhen and Beijing A-share at the current rates; it is
 * the rolling default and always reflects the latest regime
 * @details
 * <pre>
 * It is kept in sync with the latest regulatory rates; when the fees are cut again, update this
 * model's defaults (the year-named models such as TC_FixedA2015/2017 stay frozen for historical
 * backtests). Current rules:
 *   1) transfer fee: since 2022-04-29, 0.01 per mille of the turnover amount, charged in both
 *      directions on all markets (Shanghai, Shenzhen and Beijing)
 *   2) stamp duty: since 2023-08-28, 0.5 per mille, charged on sells only
 *   3) commission: charged in both directions, 5 yuan minimum
 * </pre>
 * @ingroup TradeCost
 */
class HKU_API FixedATradeCost : public FixedATradeCostBase {
    TRADE_COST_FIXEDA_SERIALIZATION

public:
    /**
     * Default constructor, it also sets the default parameter values (current rates)
     * @details
     * <pre>
     * Commission ratio, 1.8 per mille by default, i.e. 0.0018
     * Minimum commission value, 5 yuan by default
     * Stamp duty, 0.5 per mille by default, i.e. 0.0005
     * Transfer fee, 0.01 per mille of turnover by default, i.e. 0.00001
     * </pre>
     */
    FixedATradeCost();

    /**
     * @param commission commission ratio
     * @param lowestCommission minimum commission value
     * @param stamptax stamp duty
     * @param transferfee transfer fee
     */
    FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                    price_t transferfee);
    virtual ~FixedATradeCost();

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;

protected:
    /** Transfer fee by turnover value, charged on all markets */
    virtual price_t _calcTransferFee(const Stock& stock, price_t value, double num) const override;
};

} /* namespace hku */
#endif /* FIXEDATRADECOST_H_ */
