/*
 * Portfolio.h
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_PORTFOLIO_H_
#define TRADE_SYS_PORTFOLIO_PORTFOLIO_H_

#include "../allocatefunds/AllocateFundsBase.h"
#include "../selector/SelectorBase.h"

namespace hku {

/*
 * 资产组合
 * @ingroup Portfolio
 */
class HKU_API Portfolio : public enable_shared_from_this<Portfolio> {
    PARAMETER_SUPPORT

public:
    /** 默认构造函数 */
    Portfolio();

    /**
     * @brief 指定名称的构造函数
     * @param name 名称
     */
    explicit Portfolio(const string& name);

    /**
     * @brief 构造函数
     * @param tm 账户
     * @param st 选择器
     * @param af 资产分配算法
     */
    Portfolio(const TradeManagerPtr& tm, const SelectorPtr& st, const AFPtr& af);

    /** 析构函数 */
    virtual ~Portfolio();

    /** 组合名称 */
    const string& name() const;

    /** 设置组合名称 */
    void name(const string& name);

    /**
     * @brief 运行资产组合
     * @note
     * 由于各个组件可能存在参数变化的情况，无法自动感知判断是否需要重新计算，此时需要手工指定强制计算
     * @param query 查询条件
     * @param force 是否强制重计算
     */
    void run(const KQuery& query, bool force = false);

    /** 修改查询条件 */
    void setQuery(const KQuery& query);

    /** 获取查询条件 */
    const KQuery& getQuery() const;

    /** 获取账户 */
    TMPtr getTM() const;

    /** 设置账户 */
    void setTM(const TMPtr& tm);

    /** 获取选择器 */
    SEPtr getSE() const;

    /** 设置选择器 */
    void setSE(const SEPtr& se);

    /** 获取资产分配算法 */
    AFPtr getAF() const;

    /** 设置资产分配算法 */
    void setAF(const AFPtr& af);

    /** 复位操作 */
    void reset();

    typedef shared_ptr<Portfolio> PortfolioPtr;

    /** 克隆操作 */
    PortfolioPtr clone();

    /** 获取所有原型系统列表，与 SE 同 */
    const SystemList& getProtoSystemList() const;

    /** 获取所有实际运行的系统列表，与 SE 同 */
    const SystemList& getRealSystemList() const;

private:
    /** 运行前准备 */
    bool _readyForRun();

    void _runMoment(const Datetime& datetime);
    void _runMomentOnOpen(const Datetime& datetime);
    void _runMomentOnClose(const Datetime& datetime);

protected:
    string m_name;
    TMPtr m_tm;
    TMPtr m_shadow_tm;
    SEPtr m_se;
    AFPtr m_af;

    KQuery m_query;         // 关联的查询条件
    bool m_is_ready;        // 是否已做好运行准备
    bool m_need_calculate;  // 是否需要计算标志

    SystemList m_pro_sys_list;   // 所有原型系统列表，来自 SE
    SystemList m_real_sys_list;  // 所有实际运行的子系统列表

    // 用于中间计算的临时数据
    std::unordered_set<System*> m_running_sys_set;  // 当前仍在运行的子系统集合
    std::list<SYSPtr> m_running_sys_list;           // 当前仍在运行的子系统列表
    SystemList m_tmp_selected_list_on_open;
    SystemList m_tmp_selected_list_on_close;
    SystemList m_tmp_will_remove_sys;

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
        ar& BOOST_SERIALIZATION_NVP(m_tm);
        ar& BOOST_SERIALIZATION_NVP(m_shadow_tm);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_is_ready);
        ar& BOOST_SERIALIZATION_NVP(m_need_calculate);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
        ar& BOOST_SERIALIZATION_NVP(m_shadow_tm);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_is_ready);
        ar& BOOST_SERIALIZATION_NVP(m_need_calculate);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/**
 * 客户程序都应使用该指针类型
 * @ingroup Selector
 */
typedef shared_ptr<Portfolio> PortfolioPtr;
typedef shared_ptr<Portfolio> PFPtr;

HKU_API std::ostream& operator<<(std::ostream&, const Portfolio&);
HKU_API std::ostream& operator<<(std::ostream&, const PortfolioPtr&);

inline const string& Portfolio::name() const {
    return m_name;
}

inline void Portfolio::name(const string& name) {
    m_name = name;
}

inline void Portfolio::setQuery(const KQuery& query) {
    if (m_query != query) {
        m_query = query;
        m_need_calculate = true;
    }
}

inline const KQuery& Portfolio::getQuery() const {
    return m_query;
}

inline TMPtr Portfolio::getTM() const {
    return m_tm;
}

inline void Portfolio::setTM(const TMPtr& tm) {
    if (m_tm != tm) {
        m_tm = tm;
        m_need_calculate = true;
    }
}

inline SEPtr Portfolio::getSE() const {
    return m_se;
}

inline void Portfolio::setSE(const SEPtr& se) {
    if (m_se != se) {
        m_se = se;
        m_need_calculate = true;
    }
}

inline AFPtr Portfolio::getAF() const {
    return m_af;
}

inline void Portfolio::setAF(const AFPtr& af) {
    if (m_af != af) {
        m_af = af;
        m_need_calculate = true;
    }
}

inline const SystemList& Portfolio::getProtoSystemList() const {
    return m_pro_sys_list;
}

inline const SystemList& Portfolio::getRealSystemList() const {
    return m_real_sys_list;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::Portfolio> : ostream_formatter {};

template <>
struct fmt::formatter<hku::PortfolioPtr> : ostream_formatter {};
#endif

#endif /* TRADE_SYS_PORTFOLIO_PORTFOLIO_H_ */
