/*
 * AShareTradeCost.h
 *
 *  Created on: 2018年4月11日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDA2017TRADECOST_H_
#define TRADE_MANAGE_IMP_FIXEDA2017TRADECOST_H_

#include "../TradeCostBase.h"

namespace hku {

/*
 * 2017年1月1日起将对深市过户费项目单独列示，标准为成交金额0.02‰双向收取。
 */
class FixedA2017TradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedA2017TradeCost();
    virtual ~FixedA2017TradeCost();

    virtual void _checkParam(const string& name) const override;

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

#endif /* TRADE_MANAGE_IMP_FIXEDA2017TRADECOST_H_ */
