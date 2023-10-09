/*
 * SlippageBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef SLIPPAGEBASE_H_
#define SLIPPAGEBASE_H_

#include "../../KData.h"
#include "../../utilities/Parameter.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * 移滑价差算法基类
 * @ingroup Slippage
 */
class HKU_API SlippageBase : public enable_shared_from_this<SlippageBase> {
    PARAMETER_SUPPORT

public:
    SlippageBase();
    SlippageBase(const string& name);
    virtual ~SlippageBase() {}

    /** 设置交易对象 */
    void setTO(const KData& kdata);

    /** 获取交易对象 */
    KData getTO() const;

    /** 获取名称 */
    string name() const;

    /** 设置名称 */
    void name(const string& name);

    /** 复位操作 */
    void reset();

    typedef shared_ptr<SlippageBase> SlippagePtr;
    /** 克隆操作 */
    SlippagePtr clone();

    /**
     * 计算实际买入价格
     * @param datetime 买入时刻
     * @param planPrice 计划买入价格
     * @return 实际买入价格
     */
    virtual price_t getRealBuyPrice(const Datetime& datetime, price_t planPrice) = 0;

    /**
     * 计算实际卖出价格
     * @param datetime 卖出时刻
     * @param planPrice 计划卖出价格
     * @return 实际卖出价格
     */
    virtual price_t getRealSellPrice(const Datetime& datetime, price_t planPrice) = 0;

    /** 子类克隆接口 */
    virtual SlippagePtr _clone() = 0;

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类计算接口，由setTO调用 */
    virtual void _calculate() = 0;

protected:
    string m_name;
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
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SlippageBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public SlippageBase {
 *     SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Slippage
 */
#define SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SlippageBase); \
    }
#else
#define SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define SLIPPAGE_IMP(classname)                                          \
public:                                                                  \
    virtual SlippagePtr _clone() override {                              \
        return SlippagePtr(new classname());                             \
    }                                                                    \
    virtual price_t getRealBuyPrice(const Datetime&, price_t) override;  \
    virtual price_t getRealSellPrice(const Datetime&, price_t) override; \
    virtual void _calculate() override;

/**
 * 客户程序都应使用该指针类型，操作移滑价差算法
 * @ingroup Slippage
 */
typedef shared_ptr<SlippageBase> SlippagePtr;
typedef shared_ptr<SlippageBase> SPPtr;

HKU_API std::ostream& operator<<(std::ostream&, const SlippageBase&);
HKU_API std::ostream& operator<<(std::ostream&, const SlippagePtr&);

inline string SlippageBase::name() const {
    return m_name;
}

inline void SlippageBase::name(const string& name) {
    m_name = name;
}

inline KData SlippageBase::getTO() const {
    return m_kdata;
}

inline void SlippageBase::reset() {
    _reset();
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SlippageBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::SlippagePtr> : ostream_formatter {};
#endif

#endif /* SLIPPAGEBASE_H_ */
