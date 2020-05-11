/*
 * ZeroTradeCost.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef ZEROTRADECOST_H_
#define ZEROTRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

class HKU_API ZeroTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ZeroTradeCost();
    virtual ~ZeroTradeCost();

    /**
     * 计算买入成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 买入价格
     * @param num 买入数量
     * @return CostRecord 交易成本记录
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    /**
     * 计算卖出成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 卖出价格
     * @param num 卖出数量
     * @return CostRecord 交易成本记录
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    /** 子类私有变量克隆接口 */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hku */
#endif /* ZEROTRADECOST_H_ */
