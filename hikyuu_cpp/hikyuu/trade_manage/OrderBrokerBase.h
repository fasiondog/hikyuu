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
#include "../trade_sys/system/SystemPart.h"

namespace hku {

struct HKU_API BrokerPositionRecord {
    Stock stock;
    price_t number{0.0};  // 数量
    price_t money{0.0};   // 买入花费总资金

    BrokerPositionRecord() = default;
    BrokerPositionRecord(const Stock& stock, price_t number, price_t money);
    BrokerPositionRecord(const BrokerPositionRecord&) = default;
    BrokerPositionRecord(BrokerPositionRecord&& rv);

    BrokerPositionRecord& operator=(const BrokerPositionRecord&) = default;
    BrokerPositionRecord& operator=(BrokerPositionRecord&& rv);

    string str() const;
};

HKU_API std::ostream& operator<<(std::ostream& os, const BrokerPositionRecord&);

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
     * @param stoploss 预期的止损价
     * @param goalPrice 预期的目标价位
     * @param from 系统部件来源
     */
    void buy(Datetime datetime, const string& market, const string& code, price_t price, double num,
             price_t stoploss, price_t goalPrice, SystemPart from) noexcept;

    /**
     * 执行卖出操作
     * @param datetime 策略指示时间
     * @param market 市场标识
     * @param code 证券代码
     * @param price 卖出价格
     * @param num 卖出数量
     * @param stoploss 新的预期止损价
     * @param goalPrice 新的预期目标价位
     * @param from 系统部件来源
     */
    void sell(Datetime datetime, const string& market, const string& code, price_t price,
              double num, price_t stoploss, price_t goalPrice, SystemPart from) noexcept;

    /**
     * 获取当前资产信息
     * @return string json字符串
     * <pre>
     * 接口规范：
     * {
     *   "datetime": "2001-01-01 18:00:00.12345",
     *   "cash": 0.0,
     *   "positions": [
     *       {"market": "SZ", "code": "000001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
     *        "cost_price": 0.0},
     *       {"market": "SH", "code": "600001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
     *        "cost_price": 0.0},
     *    ]
     * }
     *
     * 说明：
     * cash: 当前可用资金
     * number 应该为：现有持仓 + 正在买入 - 正在卖出
     * cost_price: 每股买入成本价
     * </pre>
     */
    string getAssetInfo() noexcept;

    /**
     * 子类实现接口，执行实际买入操作
     * @param datetime 策略指示时间
     * @param market 市场标识
     * @param code 证券代码
     * @param price 买入价格
     * @param num 买入数量
     * @param stoploss 预期的止损价
     * @param goalPrice 预期的目标价位
     * @param from 系统部件来源
     */
    virtual void _buy(Datetime datetime, const string& market, const string& code, price_t price,
                      double num, price_t stoploss, price_t goalPrice, SystemPart from) = 0;

    /**
     * 子类实现接口，执行实际卖出操作
     * @param datetime 策略指示时间
     * @param market 市场标识
     * @param code 证券代码
     * @param price 卖出价格
     * @param num 卖出数量
     */
    virtual void _sell(Datetime datetime, const string& market, const string& code, price_t price,
                       double num, price_t stoploss, price_t goalPrice, SystemPart from) = 0;

    virtual string _getAssetInfo() {
        return string();
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
