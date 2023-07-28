/*
 * SelectorBase.h
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SELECTOR_SELECTORBASE_H_
#define TRADE_SYS_SELECTOR_SELECTORBASE_H_

#include "../system/System.h"
#include "../allocatefunds/AllocateFundsBase.h"
#include "../../KData.h"
#include "../../utilities/Parameter.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

class HKU_API Portfolio;

/**
 * 交易对象选择模块
 * @ingroup Selector
 */
class HKU_API SelectorBase : public enable_shared_from_this<SelectorBase> {
    PARAMETER_SUPPORT

public:
    /** 默认构造函数 */
    SelectorBase();

    /**
     * 构造函数，同时指定算法名称
     * @param name 指定名称
     */
    SelectorBase(const string& name);

    /** 析构函数 */
    virtual ~SelectorBase();

    /** 获取算法名称 */
    const string& name() const;

    /** 设置算法名称 */
    void name(const string& name);

    /**
     * 添加备选股票及其交易策略原型
     * @param stock 备选股票
     * @param protoSys 交易系统策略原型
     * @return 如果 protoSys 无效 或 stock 无效，则返回 false， 否则返回 true
     */
    bool addStock(const Stock& stock, const SystemPtr& protoSys);

    /**
     * 加入一组相同交易策略的股票
     * @note 如果存在无效的stock，则自动忽略，不会返回false
     * @param stkList 备选股票列表
     * @param protoSys 交易系统策略原型
     * @return 如果 protoSys 无效则返回false，否则返回 true
     */
    bool addStockList(const StockList& stkList, const SystemPtr& protoSys);

    /**
     * @brief 获取原型系统列表
     * @return const SystemList&
     */
    const SystemList& getProtoSystemList() const;

    /**
     * @brief 获取由 PF 实际运行的系统列表
     * @return const SystemList&
     */
    const SystemList& getRealSystemList() const;

    /**
     * @brief 复位
     * @note 复位不会清除已有的原型系统
     */
    void reset();

    /**
     * 清除已有的系统原型
     */
    void removeAll();

    typedef shared_ptr<SelectorBase> SelectorPtr;
    SelectorPtr clone();

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆接口 */
    virtual SelectorPtr _clone() = 0;

    /** 子类计算接口 */
    virtual void _calculate() = 0;

    /** 子类获取指定时刻开盘时选中的标的 */
    virtual SystemList getSelectedOnOpen(Datetime date) = 0;

    /** 子类获取指定时刻收盘时选中的标的 */
    virtual SystemList getSelectedOnClose(Datetime date) = 0;

    virtual bool isMatchAF(const AFPtr& af) = 0;

private:
    friend class HKU_API Portfolio;

    /* 仅供PF调用，由PF通知其实际运行的系统列表，并启动计算 */
    void calculate(const SystemList& sysList, const KQuery& query);

protected:
    string m_name;
    SystemList m_pro_sys_list;  // 原型系统列表
    SystemList m_real_sys_list;  // PF组合中实际运行的系统，有PF执行时设定，顺序与原型列表一一对应

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
        ar& BOOST_SERIALIZATION_NVP(m_pro_sys_list);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_pro_sys_list);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SelectorBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public SelectorBase {
 *     SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase); \
    }
#else
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define SELECTOR_IMP(classname)                                    \
public:                                                            \
    virtual SelectorPtr _clone() override {                        \
        return SelectorPtr(new classname());                       \
    }                                                              \
    virtual SystemList getSelectedOnOpen(Datetime date) override;  \
    virtual SystemList getSelectedOnClose(Datetime date) override; \
    virtual bool isMatchAF(const AFPtr& af) override;              \
    virtual void _calculate() override;

/**
 * 客户程序都应使用该指针类型
 * @ingroup Selector
 */
typedef shared_ptr<SelectorBase> SelectorPtr;
typedef shared_ptr<SelectorBase> SEPtr;

HKU_API std::ostream& operator<<(std::ostream&, const SelectorBase&);
HKU_API std::ostream& operator<<(std::ostream&, const SelectorPtr&);

inline const string& SelectorBase::name() const {
    return m_name;
}

inline void SelectorBase::name(const string& name) {
    m_name = name;
}

inline const SystemList& SelectorBase::getRealSystemList() const {
    return m_real_sys_list;
}

inline const SystemList& SelectorBase::getProtoSystemList() const {
    return m_pro_sys_list;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SelectorBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::SelectorPtr> : ostream_formatter {};
#endif

#endif /* TRADE_SYS_SELECTOR_SELECTORBASE_H_ */
