/*
 * AllocateMoney.h
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_
#define TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_

#include "../../utilities/Parameter.h"
#include "../allocatefunds/SystemWeight.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * 资产分配算法
 * @ingroup AllocateFunds
 */
class HKU_API AllocateFundsBase : public enable_shared_from_this<AllocateFundsBase> {
    PARAMETER_SUPPORT

public:
    /** 默认构造函数 */
    AllocateFundsBase();

    /**
     * 构造函数
     * @param name 算法名称
     */
    AllocateFundsBase(const string& name);

    /** 析构函数 */
    virtual ~AllocateFundsBase();

    /** 获取算法名称 */
    string name() const;

    /** 修改算法名称 */
    void name(const string& name);

    /**
     * 执行资产分配调整
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @param running_list 当前运行中的系统实例
     * @return
     */
    void adjustFunds(const Datetime& date, const SystemList& se_list,
                     const std::list<SYSPtr>& running_list);

    /** 获取交易账户 */
    TMPtr getTM();

    /** 设定交易账户 */
    void setTM(const TMPtr&);

    /** 获取关联查询条件 */
    KQuery getQuery();

    /** 设置查询条件 */
    void setQuery(KQuery query);

    /** 获取不参与资产分配的保留比例 */
    double getReservePercent();

    /**
     * 设置不参与资产分配的保留比例，该比例在执行reset时会被置为0
     * @param p 取值范围[0,1]，小于0将被强制置为0， 大于1将被置为1
     */
    void setReserverPercent(double p);

    /** 复位 */
    void reset();

    typedef shared_ptr<AllocateFundsBase> AFPtr;
    /** 克隆操作 */
    AFPtr clone();

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual AFPtr _clone() = 0;

    /**
     * 子类分配权重接口，获取实际分配资产的系统实例及其权重
     * @details 实际调用子类接口 _allocateWeight，并根据允许的最大持仓系统数参数对子类返回的
     *          系统实例及权重列表进行了截断处理
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @return
     */
    virtual SystemWeightList _allocateWeight(const Datetime& date, const SystemList& se_list) = 0;

private:
    /* 同时调整已运行中的子系统（已分配资金或已持仓） */
    void _adjust_with_running(const Datetime& date, const SystemList& se_list,
                              const std::list<SYSPtr>& running_list);

    /* 仅适用剩余资金在选中的子系统中分配资金 */
    void _adjust_without_running(const Datetime& date, const SystemList& se_list,
                                 const std::list<SYSPtr>& running_list);

    /* 计算当前的资产总值 */
    price_t _getTotalFunds(const std::list<SYSPtr>& running_list);

private:
    string m_name;
    KQuery m_query;
    int m_count;
    Datetime m_pre_date;
    TMPtr m_tm;

    double m_reserve_percent;  //保留资产比例，不参与资产分配

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        string name_str(GBToUTF8(m_name));
        ar& boost::serialization::make_nvp("name", name_str);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_count);
        ar& BOOST_SERIALIZATION_NVP(m_pre_date);
        ar& BOOST_SERIALIZATION_NVP(m_reserve_percent);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& boost::serialization::make_nvp("name", m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_count);
        ar& BOOST_SERIALIZATION_NVP(m_pre_date);
        ar& BOOST_SERIALIZATION_NVP(m_reserve_percent);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
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
 * class Drived: public AllocateFundsBase {
 *     ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                            \
    friend class boost::serialization::access;                      \
    template <class Archive>                                        \
    void serialize(Archive& ar, const unsigned int version) {       \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(AllocateFundsBase); \
    }
#else
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define ALLOCATEFUNDS_IMP(classname)   \
public:                                \
    virtual AFPtr _clone() override {  \
        return AFPtr(new classname()); \
    }                                  \
    virtual SystemWeightList _allocateWeight(const Datetime&, const SystemList&) override;

typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;
typedef shared_ptr<AllocateFundsBase> AFPtr;

HKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsBase&);
HKU_API std::ostream& operator<<(std::ostream&, const AFPtr&);

inline string AllocateFundsBase::name() const {
    return m_name;
}

inline void AllocateFundsBase::name(const string& name) {
    m_name = name;
}

inline TMPtr AllocateFundsBase::getTM() {
    return m_tm;
}

inline void AllocateFundsBase::setTM(const TMPtr& tm) {
    m_tm = tm;
}

inline KQuery AllocateFundsBase::getQuery() {
    return m_query;
}

inline void AllocateFundsBase::setQuery(KQuery query) {
    m_query = query;
}

inline double AllocateFundsBase::getReservePercent() {
    return m_reserve_percent;
}

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_ */
