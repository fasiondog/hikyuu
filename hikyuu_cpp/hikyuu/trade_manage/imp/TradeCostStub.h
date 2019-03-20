/*
 * TradeCostStub.h
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */

#ifndef TRADECOSTSTUB_H_
#define TRADECOSTSTUB_H_

#include "../TradeCostBase.h"

namespace hku {

class HKU_API TradeCostStub: public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TradeCostStub(): TradeCostBase("TestStub") {}

    virtual ~TradeCostStub() {}

    virtual CostRecord getBuyCost(const Datetime& datetime,
            const Stock& stock, price_t price, size_t num) const {
        return CostRecord(0, 0, 0, 10, 10);
    }

    virtual CostRecord getSellCost(const Datetime& datetime,
            const Stock& stock, price_t price, size_t num) const {
        return CostRecord(0, 0, 0, 20, 20);
    }

    virtual CostRecord getBorrowCashCost(const Datetime& datetime,
            price_t cash) const {
        return CostRecord(0, 0, 0, 30, 30);
    }

    virtual CostRecord getReturnCashCost(
            const Datetime& borrow_datetime,
            const Datetime& return_datetime,
            price_t cash) const {
        return CostRecord(0, 0, 0, 40, 40);
    }

    virtual CostRecord getBorrowStockCost(const Datetime& datetime,
            const Stock& stock, price_t price, size_t num) const {
        return CostRecord(0, 0, 0, 50, 50);
    }

    virtual CostRecord getReturnStockCost(
            const Datetime& borrow_datetime,
            const Datetime& return_datetime,
            const Stock& stock, price_t price, size_t num) const {
        return CostRecord(0, 0, 0, 60, 60);
    }


    /** 子类私有变量克隆接口 */
    virtual TradeCostPtr _clone() { return TradeCostPtr(new TradeCostStub); }
};


} /* namespace hku */
#endif /* TRADECOSTSTUB_H_ */
