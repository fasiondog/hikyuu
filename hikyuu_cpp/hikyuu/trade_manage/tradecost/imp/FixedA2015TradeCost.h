/*
 * FixedA2015TradeCost.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDA2015TRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDA2015TRADECOST_H_

#include "FixedATradeCostBase.h"

namespace hku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share after August 1, 2015
 * @details
 * <pre>
 * The transfer fee is charged by turnover value (0.02 per mille) on the Shanghai market only.
 * </pre>
 * @ingroup TradeCost
 */
class FixedA2015TradeCost : public FixedATradeCostBase {
    TRADE_COST_FIXEDA_SERIALIZATION

public:
    FixedA2015TradeCost();
    virtual ~FixedA2015TradeCost();

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;

protected:
    /** Transfer fee by turnover value, charged on the Shanghai market only */
    virtual price_t _calcTransferFee(const Stock& stock, price_t value, double num) const override;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_FIXEDA2015TRADECOST_H_ */
