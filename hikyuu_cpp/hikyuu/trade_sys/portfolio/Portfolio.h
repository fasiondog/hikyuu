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
    PARAMETER_SUPPORT_WITH_CHECK

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
     * @param name 组合名称
     * @param tm 账户
     * @param se 选择器
     * @param af 资产分配算法
     */
    Portfolio(const string& name, const TradeManagerPtr& tm, const SelectorPtr& se,
              const AFPtr& af);

    /** 析构函数 */
    virtual ~Portfolio();

    /** 组合名称 */
    const string& name() const;

    /** 设置组合名称 */
    void name(const string& name);

    /**
     * @brief 运行资产组合
     * @param query 查询条件, 其 KType 必须为 KQuery::DAY
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

    const SystemList& getRealSystemList() const;

    /** 复位操作 */
    void reset();

    /** 克隆操作 */
    typedef shared_ptr<Portfolio> PortfolioPtr;
    PortfolioPtr clone() const;

    /** 运行前准备 */
    void readyForRun();

    void runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust);

    /** 用于打印输出 */
    virtual string str() const;

    virtual void _reset() {}
    virtual PortfolioPtr _clone() const {
        return std::make_shared<Portfolio>();
    }

    virtual void _readyForRun() {}
    virtual void _runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) {}

private:
    void initParam();

    void _runOnMode(const DatetimeList& datelist, int adjust_cycle, const string& mode);

    void _runOnModeDelayToTradingDay(const DatetimeList& datelist, int adjust_cycle,
                                     const string& mode);

protected:
    // 跟踪打印当前TM持仓情况
    void traceMomentTM(const Datetime& date);

protected:
    string m_name;
    TMPtr m_tm;
    TMPtr m_cash_tm;  // 仅仅负责内部资金的管理（即只需要 checkout 到子账号, 从账户checkin现金）
    SEPtr m_se;
    AFPtr m_af;

    KQuery m_query;         // 关联的查询条件
    bool m_need_calculate;  // 是否需要计算标志

    SystemList m_real_sys_list;  // 所有实际运行的子系统列表

    // 用于中间计算的临时数据
    std::unordered_set<SYSPtr> m_running_sys_set;

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
        ar& BOOST_SERIALIZATION_NVP(m_cash_tm);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_query);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
        ar& BOOST_SERIALIZATION_NVP(m_cash_tm);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_query);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#define PORTFOLIO_IMP(classname)                   \
public:                                            \
    virtual PortfolioPtr _clone() const override { \
        return std::make_shared<classname>();      \
    }                                              \
    virtual void _reset() override;                \
    virtual void _readyForRun() override;          \
    virtual void _runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) override;

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
