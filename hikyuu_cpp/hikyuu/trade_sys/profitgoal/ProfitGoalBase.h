/*
 * ProfitGoal.h
 *
 *  Created on: 2013-3-7
 *      Author: fasiondog
 */

#pragma once
#ifndef PROFITGOAL_H_
#define PROFITGOAL_H_

#include "../../KData.h"
#include "../../utilities/Parameter.h"
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
 * 盈利目标策略基类
 * @details 交易前确定盈利目标，用于系统在价格达到盈利目标时执行卖出
 * @ingroup ProfitGoal
 */
class HKU_API ProfitGoalBase : public enable_shared_from_this<ProfitGoalBase> {
    PARAMETER_SUPPORT

public:
    ProfitGoalBase();
    ProfitGoalBase(const string& name);
    virtual ~ProfitGoalBase();

    /** 设置账户 */
    void setTM(const TradeManagerPtr& tm);

    /** 获取账户 */
    TradeManagerPtr getTM() const;

    /** 设置交易对象 */
    void setTO(const KData& kdata);

    /** 获取交易对象 */
    KData getTO() const;

    /** 获取名称 */
    const string& name() const;

    /** 设置名称 */
    void name(const string& name);

    /** 接收实际交易变化情况 */
    virtual void buyNotify(const TradeRecord&) {}

    /** 接收实际交易变化情况 */
    virtual void sellNotify(const TradeRecord&) {}

    /** 复位操作 */
    void reset();

    typedef shared_ptr<ProfitGoalBase> ProfitGoalPtr;
    /** 克隆接口 */
    ProfitGoalPtr clone();

    /**
     * 买入时计算目标价格
     * @param datetime 买入时间
     * @param price 买入价格
     * @return 返回Null<price_t>时，表示未限定目标; 返回0，意味着需要卖出
     */
    virtual price_t getGoal(const Datetime& datetime, price_t price) = 0;

    /** 返回0,表示未设目标 */
    virtual price_t getShortGoal(const Datetime&, price_t) {
        return 0.0;
    }

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆接口 */
    virtual ProfitGoalPtr _clone() = 0;

    /** 子类计算接口，由setTO调用 */
    virtual void _calculate() = 0;

protected:
    string m_name;
    KData m_kdata;
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
BOOST_SERIALIZATION_ASSUME_ABSTRACT(ProfitGoalBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public ProfitGoalBase {
 *     PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup ProfitGoal
 */
#define PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                         \
    friend class boost::serialization::access;                   \
    template <class Archive>                                     \
    void serialize(Archive& ar, const unsigned int version) {    \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ProfitGoalBase); \
    }
#else
#define PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define PROFITGOAL_IMP(classname)                               \
public:                                                         \
    virtual ProfitGoalPtr _clone() override {                   \
        return ProfitGoalPtr(new classname());                  \
    }                                                           \
    virtual price_t getGoal(const Datetime&, price_t) override; \
    virtual void _calculate() override;

/**
 * 客户程序都应使用该指针类型
 * @ingroup ProfitGoal
 */
typedef shared_ptr<ProfitGoalBase> ProfitGoalPtr;
typedef shared_ptr<ProfitGoalBase> PGPtr;

HKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalBase& pg);
HKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalPtr& pg);

inline void ProfitGoalBase::setTM(const TradeManagerPtr& tm) {
    m_tm = tm;
}

inline TradeManagerPtr ProfitGoalBase::getTM() const {
    return m_tm;
}

inline KData ProfitGoalBase::getTO() const {
    return m_kdata;
}

inline const string& ProfitGoalBase::name() const {
    return m_name;
}

inline void ProfitGoalBase::name(const string& name) {
    m_name = name;
}

inline void ProfitGoalBase::reset() {
    _reset();
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::ProfitGoalBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::ProfitGoalPtr> : ostream_formatter {};
#endif

#endif /* PROFITGOAL_H_ */
