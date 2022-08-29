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
    TradeCostStub() : TradeCostBase("TestStub") {}

    virtual ~TradeCostStub() {}

    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override {
        return CostRecord(0, 0, 0, 10, 10);
    }

    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override {
        return CostRecord(0, 0, 0, 20, 20);
    }

    /** 子类私有变量克隆接口 */
    virtual TradeCostPtr _clone() override {
        return TradeCostPtr(new TradeCostStub);
    }
};

} /* namespace hku */
#endif /* TRADECOSTSTUB_H_ */
