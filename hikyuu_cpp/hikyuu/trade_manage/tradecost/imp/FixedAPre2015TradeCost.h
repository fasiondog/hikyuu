/*
 * FixedAPre2015TradeCost.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDAPRE2015TRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDAPRE2015TRADECOST_H_

#include "FixedATradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share before August 1, 2015
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: both the commission and the transfer fee have a minimum value; the commission ratio is
 *   1.8 per mille (5 yuan minimum), and the stamp duty is 1 per mille
 *         The transfer fee of the Shanghai Stock Exchange is 1 per mille of the traded quantity
 *   (i.e. 1 yuan per 1000 shares), and it is counted as one yuan when it is less than 1 yuan
 * </pre>
 * @ingroup TradeCost
 */
class HKU_API FixedAPre2015TradeCost : public FixedATradeCostBase {
    TRADE_COST_FIXEDA_SERIALIZATION

public:
    FixedAPre2015TradeCost();

    /**
     * @param commission commission ratio
     * @param lowestCommission minimum commission value
     * @param stamptax stamp duty
     * @param transferfee transfer fee (per share)
     * @param lowestTransferfee minimum transfer fee
     */
    FixedAPre2015TradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                           price_t transferfee, price_t lowestTransferfee);
    virtual ~FixedAPre2015TradeCost();

    virtual void _checkParam(const string& name) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;

protected:
    /** Per-share transfer fee, charged on the Shanghai market only */
    virtual price_t _calcTransferFee(const Stock& stock, price_t value, double num) const override;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_FIXEDAPRE2015TRADECOST_H_ */
