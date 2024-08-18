/*
 * OrderBrokerBase.h
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_ORDERBROKERBASE_H_
#define TRADE_MANAGE_ORDERBROKERBASE_H_

#include "../DataType.h"
#include "../utilities/Parameter.h"

namespace hku {

/**
 * 订单代理基类，实现实际的订单操作及程序化的订单。
 * @details 可通过向 TradeManager.regBroker 向 TradeManager 注册多个订单代理实例。
 *          这些订单代理可执行额外的买入/卖出动作，如邮件订单代理，在 TradeManager 发出
 *          买入/卖出指令时， 邮件订单代理可以发送邮件。
 *
 *          默认情况下，TradeManager会在执行买入/卖出操作时，调用订单代理执行代理的买入/卖出动作，
 *          但这样在实盘操作时会存在问题。因为系统在计算信号指示时，需要回溯历史数据才能得到最新的信号，
 *          这样TradeManager会在历史时刻就执行买入/卖出操作。此时如果订单代理本身没有对发出
 *          买入/卖出指令的时刻进行控制，会导致代理发送错误的指令 。因此，需要指定在某一个时刻之后，
 *          才允许执行订单代理的买入/卖出操作。TradeManager的属性 brokeLastDatetime 即用于
 *          指定该时刻。
 * @ingroup OrderBroker
 */
class HKU_API OrderBrokerBase {
    PARAMETER_SUPPORT

public:
    OrderBrokerBase();
    OrderBrokerBase(const string& name);
    virtual ~OrderBrokerBase();

    /** 获取名称 */
    const string& name() const;

    /** 设置名称 */
    void name(const string& name);

    /**
     * 执行买入操作
     * @param datetime 策略指示时间
     * @param market 市场标识
     * @param code 证券代码
     * @param price 买入价格
     * @param num 买入数量
     * @return 委托单号，否则返回空字符串
     */
    string buy(Datetime datetime, const string& market, const string& code, price_t price,
               double num) noexcept;

    /**
     * 执行卖出操作
     * @param datetime 策略指示时间
     * @param market 市场标识
     * @param code 证券代码
     * @param price 卖出价格
     * @param num 卖出数量
     * @return 委托单号，否则返回空字符串
     */
    string sell(Datetime datetime, const string& market, const string& code, price_t price,
                double num) noexcept;

    /**
     * 获取当前可用资金
     */
    price_t cash() noexcept;

    vector<Parameter> position() noexcept;

    /**
     * 子类实现接口，执行实际买入操作
     * @param datetime 策略指示时间
     * @param market 市场标识
     * @param code 证券代码
     * @param price 买入价格
     * @param num 买入数量
     * @return 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
     */
    virtual string _buy(Datetime datetime, const string& market, const string& code, price_t price,
                        double num) = 0;

    /**
     * 子类实现接口，执行实际卖出操作
     * @param datetime 策略指示时间
     * @param market 市场标识
     * @param code 证券代码
     * @param price 卖出价格
     * @param num 卖出数量
     * @return 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
     */
    virtual string _sell(Datetime datetime, const string& market, const string& code, price_t price,
                         double num) = 0;

    /**
     * 子类获取当前可用现金接口
     */
    virtual price_t _cash() {
        return 0.0;
    }

    /**
     * 子类获取持仓信息实现
     * @return vector<string> json 字符串组成的持仓信息列表
     * <pre>
     * 其中：market, code, num 为必须
     * 示例：
     * [{"market": "SZ",
     *   "code": "000001",
     *   "num": 100,
     *   "buy_frozen_num", 0,
     *   "sell_frozen_num", 0
     * }]
     * </pre
     */
    virtual vector<string> _position() {
        return vector<string>();
    }

protected:
    string m_name;
};

/**
 * 客户程序应使用此类型进行实际操作
 * @ingroup OrderBroker
 */
typedef shared_ptr<OrderBrokerBase> OrderBrokerPtr;

/** @ingroup OrderBroker */
HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerBase&);

/** @ingroup OrderBroker */
HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerPtr&);

inline const string& OrderBrokerBase::name() const {
    return m_name;
}

inline void OrderBrokerBase::name(const string& name) {
    m_name = name;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::OrderBrokerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::OrderBrokerPtr> : ostream_formatter {};
#endif

#endif /* TRADE_MANAGE_ORDERBROKERBASE_H_ */
