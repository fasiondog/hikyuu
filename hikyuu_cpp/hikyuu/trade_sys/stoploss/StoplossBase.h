/*
 * StoplossBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef STOPLOSSBASE_H_
#define STOPLOSSBASE_H_

#include "../../KData.h"
// #include "../../utilities/Parameter.h"
#include "../../trade_manage/TradeManager.h"

namespace hku {

/**
 * 止损/止赢策略基类
 * @details 负责向系统提供当前计划交易的预期止损价
 * @ingroup Stoploss
 */
class HKU_API StoplossBase : public enable_shared_from_this<StoplossBase> {
    PARAMETER_SUPPORT

public:
    StoplossBase();
    StoplossBase(const string& name);
    virtual ~StoplossBase();

    /** 获取名称 */
    const string& name() const;

    /** 设置名称 */
    void name(const string& name);

    /** 设置交易管理实例 */
    void setTM(const TradeManagerPtr& tm);

    /** 获取交易管理实例 */
    TradeManagerPtr getTM() const;

    /** 设置交易对象 */
    void setTO(const KData& kdata);

    /** 获取交易对象  */
    KData getTO() const;

    /** 复位操作 */
    void reset();

    typedef shared_ptr<StoplossBase> StoplossPtr;
    /** 克隆操作 */
    StoplossPtr clone();

    /**
     * 获取本次预期交易（买入）时的计划止损价格，如果不存在止损价，则返回0。
     * 用于系统在交易执行前向止损策略模块查询本次交易的计划止损价。
     * @param datetime 交易时间
     * @param price 计划买入的价格
     * @note 一般情况下，止损/止赢的算法可以互换，但止损的getPrice可以传入计划交易的
     *       价格，比如以买入价格的30%做为止损。而止赢则不考虑传入的price参数，即认为
     *       price为0.0。实际上，即使止损也不建议使用price参数，如可以使用前日最低价
     *       的30%作为止损，则不需要考虑price参数
     */
    virtual price_t getPrice(const Datetime& datetime, price_t price) = 0;

    /**
     * 获取本次预期交易（卖空）时的计划止损价格，如果不存在止损价，则返回0。
     * 用于系统在交易执行前向止损策略模块查询本次交易的计划止损价。
     * @param datetime 交易日期
     * @param price 计划交易的价格
     * @note 默认实现时，和getPrice返回结果相同
     */
    virtual price_t getShortPrice(const Datetime& datetime, price_t price) {
        return getPrice(datetime, price);
    }

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆接口 */
    virtual StoplossPtr _clone() = 0;

    /** 子类计算接口，由setTO调用 */
    virtual void _calculate() = 0;

protected:
    string m_name;
    TradeManagerPtr m_tm;
    KData m_kdata;

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
        // m_kdata都是系统运行时临时设置，不需要序列化
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        // m_kdata都是系统运行时临时设置，不需要序列化
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(StoplossBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public StoplossBase {
 *     STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Stoploss
 */
#define STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(StoplossBase); \
    }
#else
#define STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define STOPLOSS_IMP(classname, str_name)    \
public:                                      \
    virtual StoplossPtr _clone() override {  \
        return StoplossPtr(new classname()); \
    }                                        \
    virtual void _calculate() override;      \
    virtual price_t getPrice(const Datetime&, price_t) override;

/**
 * 客户程序都应使用该指针类型，操作止损策略实例
 * @ingroup StopLoss
 */
typedef shared_ptr<StoplossBase> StoplossPtr;
typedef shared_ptr<StoplossBase> STPtr;
typedef shared_ptr<StoplossBase> TakeProfitPtr;
typedef shared_ptr<StoplossBase> TPPtr;

HKU_API std::ostream& operator<<(std::ostream& os, const StoplossBase&);
HKU_API std::ostream& operator<<(std::ostream& os, const StoplossPtr&);

inline const string& StoplossBase::name() const {
    return m_name;
}

inline void StoplossBase::name(const string& name) {
    m_name = name;
}

inline TradeManagerPtr StoplossBase::getTM() const {
    return m_tm;
}

inline void StoplossBase::setTM(const TradeManagerPtr& tm) {
    m_tm = tm;
}

inline KData StoplossBase::getTO() const {
    return m_kdata;
}

inline void StoplossBase::reset() {
    _reset();
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::StoplossBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::StoplossPtr> : ostream_formatter {};
#endif

#endif /* STOPLOSSBASE_H_ */
