/*
 * TradeCostBase.h
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADECOSTBASE_H_
#define TRADECOSTBASE_H_

#include "../Stock.h"
#include "../utilities/Parameter.h"
#include "CostRecord.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>

#if HKU_SUPPORT_XML_ARCHIVE
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif /* HKU_SUPPORT_XML_ARCHIVE */

#if HKU_SUPPORT_TEXT_ARCHIVE
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif /* HKU_SUPPORT_TEXT_ARCHIVE */

#if HKU_SUPPORT_BINARY_ARCHIVE
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif /* HKU_SUPPORT_BINARY_ARCHIVE */

#include <boost/serialization/export.hpp>
#endif /* HKU_SUPPORT_SERIALIZATION */

namespace hku {

/**
 * 交易成本算法接口基类
 * @ingroup TradeCost
 */
class HKU_API TradeCostBase {
    PARAMETER_SUPPORT

public:
    TradeCostBase(const string& name);
    virtual ~TradeCostBase();

    typedef shared_ptr<TradeCostBase> TradeCostPtr;
    /** 克隆操作   */
    TradeCostPtr clone();

    /** 获取名称 */
    const string& name() const {
        return m_name;
    }

    /**
     * 计算买入成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 买入价格
     * @param num 买入数量
     * @return CostRecord 交易成本记录
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const = 0;

    /**
     * 计算卖出成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 卖出价格
     * @param num 卖出数量
     * @return CostRecord 交易成本记录
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const = 0;

    /**
     * 计算借入现金花费的成本
     * @param datetime 借入日期
     * @param cash 借入的资金
     */
    virtual CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const {
        return CostRecord();
    }

    /**
     * 计算归还融资成本
     * @param borrow_datetime 资金借入日期
     * @param return_datetime 归还日期
     * @param cash 归还金额
     */
    virtual CostRecord getReturnCashCost(const Datetime& borrow_datetime,
                                         const Datetime& return_datetime, price_t cash) const {
        return CostRecord();
    }

    /**
     * 计算融劵借入成本
     * @param datetime 融劵日期
     * @param stock 借入的对象
     * @param price 每股价格
     * @param num 借入的数量
     */
    virtual CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const {
        return CostRecord();
    }

    /**
     * 计算融劵归还成本
     * @param borrow_datetime 借入日期
     * @param return_datetime 归还日期
     * @param stock 归还的对象
     * @param price 归还时每股价格
     * @param num 归还的数量
     */
    virtual CostRecord getReturnStockCost(const Datetime& borrow_datetime,
                                          const Datetime& return_datetime, const Stock& stock,
                                          price_t price, double num) const {
        return CostRecord();
    }

    /** 继承子类必须实现私有变量的克隆接口 */
    virtual TradeCostPtr _clone() = 0;

private:
    string m_name;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(TradeCostBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class DrivedCost: public TradeCostBase {
 *     TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     DrivedCost();
 *     ...
 * };
 * @endcode
 * @ingroup TradeCost
 */
#define TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                        \
    friend class boost::serialization::access;                  \
    template <class Archive>                                    \
    void serialize(Archive& ar, const unsigned int version) {   \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeCostBase); \
    }
#else
#define TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * 交易成本算法指针
 * @ingroup TradeCost
 */
typedef shared_ptr<TradeCostBase> TradeCostPtr;

HKU_API std::ostream& operator<<(std::ostream&, const TradeCostBase&);
HKU_API std::ostream& operator<<(std::ostream&, const TradeCostPtr&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TradeCostBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::TradeCostPtr> : ostream_formatter {};
#endif

#endif /* TRADECOSTBASE_H_ */
