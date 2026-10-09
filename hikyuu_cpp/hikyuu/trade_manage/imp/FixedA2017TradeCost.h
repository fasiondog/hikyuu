/*
 * FixedA2017TradeCost.h
 *
 *  Created on: 2018-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDA2017TRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDA2017TRADECOST_H_

#include "FixedATradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share after January 1, 2017
 * @details
 * <pre>
 * From January 1, 2017 the transfer fee item of the Shenzhen market is listed separately, with the
 * standard of 0.02 per mille of the turnover amount charged in both directions (Shanghai and
 * Shenzhen).
 * </pre>
 * @ingroup TradeCost
 */
class FixedA2017TradeCost : public FixedATradeCostBase {
    TRADE_COST_FIXEDA_SERIALIZATION

public:
    FixedA2017TradeCost();
    virtual ~FixedA2017TradeCost();

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;

protected:
    /** Transfer fee by turnover value, charged on all markets */
    virtual price_t _calcTransferFee(const Stock& stock, price_t value, double num) const override;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_FIXEDA2017TRADECOST_H_ */
