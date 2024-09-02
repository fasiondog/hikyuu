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
#include "../selector/SystemWeight.h"

namespace hku {

/**
 * 资产分配调整算法
 * @details 根据资产市值对资产比例进行分配调整。单纯的资金调整，请使用资金管理算法。
 * @ingroup AllocateFunds
 */
class HKU_API AllocateFundsBase : public enable_shared_from_this<AllocateFundsBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    /** 默认构造函数 */
    AllocateFundsBase();
    AllocateFundsBase(const AllocateFundsBase&) = default;

    /**
     * 构造函数
     * @param name 算法名称
     */
    explicit AllocateFundsBase(const string& name);

    /** 析构函数 */
    virtual ~AllocateFundsBase();

    /** 获取算法名称 */
    const string& name() const;

    /** 修改算法名称 */
    void name(const string& name);

    /**
     * 执行资产分配调整，仅供 PF 调用
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @param running_list 当前运行中的系统实例
     * @return 需延迟执行卖出操作的系统列表，其中权重为相应需卖出的数量
     */
    SystemWeightList adjustFunds(const Datetime& date, const SystemWeightList& se_list,
                                 const std::unordered_set<SYSPtr>& running_list);

    /** 获取交易账户 */
    const TMPtr& getTM() const;

    /** 设定交易账户，由 PF 设定 */
    void setTM(const TMPtr&);

    /** 设置 Portfolio 的影子账户, 仅由 Portfolio 调用 */
    void setCashTM(const TMPtr&);

    const TMPtr& getCashTM(const TMPtr&) const;

    /** 获取关联查询条件 */
    const KQuery& getQuery() const;

    /** 设置查询条件， 由 PF 设定 */
    void setQuery(const KQuery& query);

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
     * @details 实际调用子类接口 _allocateWeight
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @return 子类只需要返回每个系统的相对比例即可
     */
    virtual SystemWeightList _allocateWeight(const Datetime& date,
                                             const SystemWeightList& se_list) = 0;

public:
    /*
     * 内部函数，仅为测试需要设置为 public。
     * 对由子类分配的计划权重根据内部参数设置进行调整
     */
    static void adjustWeight(SystemWeightList& sw_list, double can_allocate_weight,
                             bool auto_adjust, bool ignore_zero);

private:
    void initParam();

    /* 同时调整已运行中的子系统（已分配资金或已持仓） */
    SystemWeightList _adjust_with_running(const Datetime& date, const SystemWeightList& se_list,
                                          const std::unordered_set<SYSPtr>& running_list);

    /* 不调整已在运行中的子系统 */
    void _adjust_without_running(const Datetime& date, const SystemWeightList& se_list,
                                 const std::unordered_set<SYSPtr>& running_list);

    /* 检查分配的权重是否在 0 和 1 之间，如果存在错误，抛出异常，仅在 trace 时生效*/
    void _check_weight(const SystemWeightList&);

private:
    string m_name;    // 组件名称
    KQuery m_query;   // 查询条件
    TMPtr m_tm;       // 运行期由PF设定，PF的实际账户
    TMPtr m_cash_tm;  // 运行期由PF设定，tm 的影子账户，由于协调分配资金

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
        ar& BOOST_SERIALIZATION_NVP(m_query);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_query);
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

#define ALLOCATEFUNDS_IMP(classname)          \
public:                                       \
    virtual AFPtr _clone() override {         \
        return std::make_shared<classname>(); \
    }                                         \
    virtual SystemWeightList _allocateWeight(const Datetime&, const SystemWeightList&) override;

typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;
typedef shared_ptr<AllocateFundsBase> AFPtr;

HKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsBase&);
HKU_API std::ostream& operator<<(std::ostream&, const AFPtr&);

inline const string& AllocateFundsBase::name() const {
    return m_name;
}

inline void AllocateFundsBase::name(const string& name) {
    m_name = name;
}

inline const TMPtr& AllocateFundsBase::getTM() const {
    return m_tm;
}

inline void AllocateFundsBase::setTM(const TMPtr& tm) {
    m_tm = tm;
}

inline void AllocateFundsBase::setCashTM(const TMPtr& tm) {
    m_cash_tm = tm;
}

inline const TMPtr& AllocateFundsBase::getCashTM(const TMPtr&) const {
    return m_cash_tm;
}

inline const KQuery& AllocateFundsBase::getQuery() const {
    return m_query;
}

inline void AllocateFundsBase::setQuery(const KQuery& query) {
    m_query = query;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::AllocateFundsBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::AFPtr> : ostream_formatter {};
#endif

#endif /* TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_ */
