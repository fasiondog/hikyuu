/*
 * TradeCostStub.h
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADECOSTSTUB_H_
#define TRADECOSTSTUB_H_

#include "../TradeCostBase.h"

namespace hku {

class HKU_API TradeCostStub : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TradeCostStub();

    virtual ~TradeCostStub();

    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    virtual CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const override;

    virtual CostRecord getReturnCashCost(const Datetime& borrow_datetime,
                                         const Datetime& return_datetime,
                                         price_t cash) const override;

    virtual CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const override;

    virtual CostRecord getReturnStockCost(const Datetime& borrow_datetime,
                                          const Datetime& return_datetime, const Stock& stock,
                                          price_t price, double num) const override;

    /** 子类私有变量克隆接口 */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hku */
#endif /* TRADECOSTSTUB_H_ */
