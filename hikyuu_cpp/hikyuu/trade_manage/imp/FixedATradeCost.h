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
 * 沪深A股交易成本算法,计算每次买入或卖出的成本
 * @details
 * <pre>
 * 计算规则为：
 *   1）上证交易所
 *      买入：佣金＋过户费
 *      卖出：佣金＋过户费＋印花税
 *   2）深证交易所：
 *      买入：佣金
 *      卖出：佣金＋印花税
 *   其中：佣金和过户费均有最低值，当前佣金比例为千分之1.8（最低5元），印花税为千分之一
 *         上证过户费为交易数量的千分之一，不足1元，按一元计
 * </pre>
 */
class HKU_API FixedATradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    /**
     * 默认构造函数，同时设置默认参数值
     * @details
     * <pre>
     * 佣金比例，默认千分之1.8，即0.0018
     * 最低佣金值，默认5元
     * 印花税，默认千分之一，即0.001
     * 过户费，默认每股千分之一，即0.001
     * 最低过户费，默认1元
     * </pre>
     */
    FixedATradeCost();

    /**
     * @param commission 佣金比例
     * @param lowestCommission 最低佣金值
     * @param stamptax 印花税
     * @param transferfee 过户费
     * @param lowestTransferfee 最低过户费
     */
    FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                    price_t transferfee, price_t lowestTransferfee);
    virtual ~FixedATradeCost();

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
#endif /* FIXEDATRADECOST_H_ */
