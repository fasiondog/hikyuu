/*
 * SignalBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef SIGNALBASE_H_
#define SIGNALBASE_H_

#include <set>
#include "../../KData.h"
#include "../../utilities/Parameter.h"
#include "../../trade_manage/TradeManager.h"
#include "../../serialization/Datetime_serialization.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * 信号指示器基类
 * @ingroup Signal
 */
class HKU_API SignalBase : public enable_shared_from_this<SignalBase> {
    PARAMETER_SUPPORT

public:
    SignalBase();
    SignalBase(const string& name);
    virtual ~SignalBase();

    /**
     * 指定时刻是否可以买入
     * @param datetime 指定时刻
     * @return true 可以买入 | false 不可买入
     */
    bool shouldBuy(const Datetime& datetime) const;

    /**
     * 指定时刻是否可以卖出
     * @param datetime 指定时刻
     * @return true 可以卖出 | false 不可卖出
     */
    bool shouldSell(const Datetime& datetime) const;

    /**
     * 下一时刻是否可以买入，相当于最后时刻是否指示买入
     */
    bool nextTimeShouldBuy() const;

    /**
     * 下一时刻是否可以卖出，相当于最后时刻是否指示卖出
     */
    bool nextTimeShouldSell() const;

    /** 获取所有买入指示日期列表 */
    DatetimeList getBuySignal() const;

    /** 获取所有卖出指示日期列表 */
    DatetimeList getSellSignal() const;

    /**
     * 加入买入信号，在_calculate中调用
     * @param datetime 发生买入信号的日期
     */
    void _addBuySignal(const Datetime& datetime);

    /**
     * 加入卖出信号，在_calculate中调用
     * @param datetime
     */
    void _addSellSignal(const Datetime& datetime);

    /**
     * 指定交易对象，指K线数据
     * @param kdata 指定的交易对象
     */
    void setTO(const KData& kdata);

    /**
     * 获取交易对象
     * @return 交易对象(KData)
     */
    KData getTO() const;

    /** 复位操作 */
    void reset();

    typedef shared_ptr<SignalBase> SignalPtr;
    /** 克隆操作 */
    SignalPtr clone();

    /** 获取名称 */
    string name() const;

    /** 设置名称 */
    void name(const string& name);

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆接口 */
    virtual SignalPtr _clone() = 0;

    /** 子类计算接口，在setTO中调用 */
    virtual void _calculate() = 0;

protected:
    string m_name;
    KData m_kdata;
    /* 多头持仓 */
    bool m_hold_long;
    /* 空头持仓 */
    bool m_hold_short;

    // 用 set 保存，以便获取是能保持顺序
    std::set<Datetime> m_buySig;
    std::set<Datetime> m_sellSig;

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
        ar& BOOST_SERIALIZATION_NVP(m_hold_long);
        ar& BOOST_SERIALIZATION_NVP(m_hold_short);
        ar& BOOST_SERIALIZATION_NVP(m_buySig);
        ar& BOOST_SERIALIZATION_NVP(m_sellSig);
        // m_kdata都是系统运行时临时设置，不需要序列化
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_hold_long);
        ar& BOOST_SERIALIZATION_NVP(m_hold_short);
        ar& BOOST_SERIALIZATION_NVP(m_buySig);
        ar& BOOST_SERIALIZATION_NVP(m_sellSig);
        // m_kdata都是系统运行时临时设置，不需要序列化
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SignalBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public SignalBase {
 *     SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Signal
 */
#define SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION                \
private:                                                      \
    friend class boost::serialization::access;                \
    template <class Archive>                                  \
    void serialize(Archive& ar, const unsigned int version) { \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);  \
    }
#else
#define SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define SIGNAL_IMP(classname)              \
public:                                    \
    virtual SignalPtr _clone() override {  \
        return SignalPtr(new classname()); \
    }                                      \
    virtual void _calculate() override;

/**
 * 客户程序都应使用该指针类型，操作信号指示器
 * @ingroup Signal
 */
typedef shared_ptr<SignalBase> SignalPtr;
typedef shared_ptr<SignalBase> SGPtr;

HKU_API std::ostream& operator<<(std::ostream&, const SignalBase&);
HKU_API std::ostream& operator<<(std::ostream&, const SignalPtr&);

inline KData SignalBase::getTO() const {
    return m_kdata;
}

inline string SignalBase::name() const {
    return m_name;
}

inline void SignalBase::name(const string& name) {
    m_name = name;
}

inline bool SignalBase::shouldBuy(const Datetime& datetime) const {
    return m_buySig.count(datetime) ? true : false;
}

inline bool SignalBase::shouldSell(const Datetime& datetime) const {
    return m_sellSig.count(datetime) ? true : false;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SignalBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::SignalPtr> : ostream_formatter {};
#endif

#endif /* SIGNALBASE_H_ */
