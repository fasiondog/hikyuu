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
 * 订单代理基类，实现实际的订单操作，及程序化的订单
 * @ingroup TradeManagerClass
 */
class HKU_API OrderBrokerBase {
public:
    OrderBrokerBase();
    virtual ~OrderBrokerBase();

    void buy(const string& code, price_t price, int num);
    void sell(const string& code, price_t price, int num);

    virtual void _buy(const string& code, price_t price, int num) = 0;
    virtual void _sell(const string& code, price_t price, int num) = 0;
};

/**
 * 客户程序应使用此类型进行实际操作
 * @ingroup TradeManagerClass
 */
typedef shared_ptr<OrderBrokerBase> OrderBrokerPtr;

} /* namespace hku */

#endif /* TRADE_MANAGE_ORDERBROKERBASE_H_ */
