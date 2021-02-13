/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-13
 *     Author: fasiondog
 */

#pragma once

#include "../utilities/Parameter.h"
#include "TradeRecord.h"
#include "PositionRecord.h"
#include "BorrowRecord.h"
#include "FundsRecord.h"
#include "LoanRecord.h"
#include "OrderBrokerBase.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#endif

namespace hku {

class HKU_API TradeManagerBase {
    PARAMETER_SUPPORT

public:
    TradeManagerBase();

    /** 从哪个时刻开始启动订单代理进行下单操作   */
    Datetime getBrokerLastDatetime() const {
        return m_broker_last_datetime;
    }

    /** 设置开始订单代理操作的时刻 */
    void setBrokerLastDatetime(const Datetime& date) {
        m_broker_last_datetime = date;
    }

    /** 复位，清空交易、持仓记录 */
    virtual void reset() {
        m_broker_last_datetime = Datetime::now();
        HKU_WARN("The subclass does not implement a reset method");
    }

    // shared_ptr<TradeManager> clone();

    /**
     * 注册订单代理
     * @param broker 订单代理实例
     */
    virtual void regBroker(const OrderBrokerPtr& broker) {
        HKU_WARN("The subclass does not implement a regBroker method");
    }

    /**
     * 清空已注册的订单代理
     */
    virtual void clearBroker() {
        HKU_WARN("The subclass does not implement a clearBroker method");
    }

    /**
     * 获取指定对象的保证金比率
     * @param datetime 日期
     * @param stock 指定对象
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement a getMarginRate method");
        return 0.0;
    }

    /** 账户名称 */
    virtual string name() const {
        HKU_WARN("The subclass does not implement this method");
        return "";
    }

    /** 设置账户名称 */
    virtual void name(const string& name) {
        HKU_WARN("The subclass does not implement this method");
    }

    /** 初始资金 */
    virtual price_t initCash() const {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** 账户建立日期 */
    virtual Datetime initDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /** 第一笔买入交易发生日期，如未发生交易返回Null<Datetime>() */
    virtual Datetime firstDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /** 最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期 */
    virtual Datetime lastDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

protected:
    Datetime m_broker_last_datetime;  // 当前启动运行时间

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_broker_last_datetime);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_broker_last_datetime);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(TradeManagerBase)
#endif

}  // namespace hku