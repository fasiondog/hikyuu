/*
 * MoneyManagerBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef MONEYMANAGERBASE_H_
#define MONEYMANAGERBASE_H_

#include "../../utilities/Parameter.h"
#include "../system/SystemPart.h"
#include "../../trade_manage/TradeManager.h"

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
 * 资金管理基类
 * @ingroup MoneyManager
 */
class HKU_API MoneyManagerBase : public enable_shared_from_this<MoneyManagerBase> {
    PARAMETER_SUPPORT

public:
    MoneyManagerBase();
    MoneyManagerBase(const string& name);
    virtual ~MoneyManagerBase();

    /** 获取名称 */
    const string& name() const {
        return m_name;
    }

    /** 设置名称 */
    void name(const string& name) {
        m_name = name;
    }

    /** 复位 */
    void reset() {
        _reset();
    }

    /**
     * 设定交易账户
     * @param tm 指定的交易账户
     */
    void setTM(const TradeManagerPtr& tm) {
        m_tm = tm;
    }

    /**
     * 获取交易账户
     * @return
     */
    TradeManagerPtr getTM() const {
        return m_tm;
    }

    /** 设置查询条件 */
    void setQuery(const KQuery& query) {
        m_query = query;
    }

    /** 获取交易的K线类型 */
    const KQuery& getQuery() const {
        return m_query;
    }

    typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
    /** 克隆操作 */
    MoneyManagerPtr clone();

    /** 接收实际交易变化情况，一般存在多次增减仓的情况才需要重载 */
    virtual void buyNotify(const TradeRecord&);

    /** 接收实际交易变化情况，一般存在多次增减仓的情况才需要重载 */
    virtual void sellNotify(const TradeRecord&);

    /**
     * 获取指定交易对象可卖出的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param risk 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
     * @param from 信号来源
     * @note 默认实现返回 MAX_DOUBLE 卖出全部; 多次减仓才需要实现该接口
     */
    double getSellNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                         SystemPart from);

    /**
     * 获取指定交易对象可卖空的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param from 信号来源
     * @param risk 承担的交易风险，如果为Null<price_t>，表示不设损失上限
     */
    double getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                              price_t risk, SystemPart from);

    /**
     * 获取指定交易对象空头回补的买入数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param from 信号来源
     * @param risk 承担的交易风险，如果为Null<price_t>，表示不设损失上限
     */
    double getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                             price_t risk, SystemPart from);

    /**
     * 获取指定交易对象可买入的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param from 信号来源
     * @param risk 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
     */
    double getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                        SystemPart from);

    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                 price_t risk, SystemPart from) = 0;

    virtual double _getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                  price_t risk, SystemPart from);

    virtual double _getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from);

    virtual double _getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                      price_t risk, SystemPart from);

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual MoneyManagerPtr _clone() = 0;

protected:
    string m_name;
    KQuery m_query;
    TradeManagerPtr m_tm;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        // m_query、m_tm都是系统运行时临时设置，不需要序列化
        // ar & BOOST_SERIALIZATION_NVP(m_query);
        // ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MoneyManagerBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public MoneyManagerBase {
 *     MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MoneyManager
 */
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                           \
    friend class boost::serialization::access;                     \
    template <class Archive>                                       \
    void serialize(Archive& ar, const unsigned int version) {      \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase); \
    }
#else
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * 客户程序都应使用该指针类型
 * @ingroup MoneyManager
 */
typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
typedef shared_ptr<MoneyManagerBase> MMPtr;

#define MONEY_MANAGER_IMP(classname)                                                          \
public:                                                                                       \
    virtual MoneyManagerPtr _clone() override {                                               \
        return MoneyManagerPtr(new classname());                                              \
    }                                                                                         \
    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price, \
                                 price_t risk, SystemPart from) override;

HKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerBase&);
HKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerPtr&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MoneyManagerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::MoneyManagerPtr> : ostream_formatter {};
#endif

#endif /* MONEYMANAGERBASE_H_ */
