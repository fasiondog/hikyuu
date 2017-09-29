/*
 * OrderBrokerBase.h
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#ifndef TRADE_MANAGE_ORDERBROKERBASE_H_
#define TRADE_MANAGE_ORDERBROKERBASE_H_

#include "../DataType.h"

namespace hku {

/**
 * 订单代理基类，实现实际的订单操作及程序化的订单
 * @ingroup TradeManagerClass
 */
class HKU_API OrderBrokerBase {
public:
    OrderBrokerBase();
    virtual ~OrderBrokerBase();

    /**
     * 执行买入操作
     * @param code 证券代码
     * @param price 买入价格
     * @param num 买入数量
     * @return 操作执行的时刻
     */
    Datetime buy(const string& code, price_t price, int num);

    /**
     * 执行卖出操作
     * @param code 证券代码
     * @param price 卖出价格
     * @param num 卖出数量
     * @return 操作执行的时刻
     */
    Datetime sell(const string& code, price_t price, int num);

    /**
     * 执行实际买入操作
     * @param code 证券代码
     * @param price 买入价格
     * @param num 买入数量
     */
    virtual void _buy(const string& code, price_t price, int num) = 0;

    /**
     * 执行实际卖出操作
     * @param code 证券代码
     * @param price 卖出价格
     * @param num 卖出数量
     */
    virtual void _sell(const string& code, price_t price, int num) = 0;
};

/**
 * 客户程序应使用此类型进行实际操作
 * @ingroup TradeManagerClass
 */
typedef shared_ptr<OrderBrokerBase> OrderBrokerPtr;

} /* namespace hku */

#endif /* TRADE_MANAGE_ORDERBROKERBASE_H_ */
