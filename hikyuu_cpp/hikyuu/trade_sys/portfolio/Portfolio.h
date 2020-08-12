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

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/*
 * 资产组合
 * @ingroup Portfolio
 */
class HKU_API Portfolio {
    PARAMETER_SUPPORT

public:
    Portfolio();
    Portfolio(const string& name);
    Portfolio(const TradeManagerPtr& tm, const SelectorPtr& st, const AFPtr& af);
    virtual ~Portfolio();

    const string& name() const {
        return m_name;
    }
    void name(const string& name) {
        m_name = name;
    }

    bool isReady() const {
        return m_is_ready;
    }

    bool readyForRun();
    void run(const KQuery& query);
    void runMoment(const Datetime& datetime);

    void setQuery(const KQuery& query) {
        m_query = query;
    }

    KQuery getQuery() const {
        return m_query;
    }

    TMPtr getTM() {
        return m_tm;
    }
    SEPtr getSE() {
        return m_se;
    }
    AFPtr getAF() {
        return m_af;
    }

    void setTM(const TMPtr& tm) {
        m_tm = tm;
    }
    void setSE(const SEPtr& se) {
        m_se = se;
    }
    void setAF(const AFPtr& af) {
        m_af = af;
    }

    SystemList getAllSystem() const {
        SystemList result;
        for (auto& sys : m_all_sys_set) {
            result.push_back(sys);
        }
        return result;
    }

    void reset();

    typedef shared_ptr<Portfolio> PortfolioPtr;
    PortfolioPtr clone();

    /**
     * 获取资产组合账户当前时刻的资产详情
     * @param ktype 日期的类型
     * @return 资产详情
     */
    FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const;

    /**
     * 获取指定时刻的资产市值详情
     * @param datetime 必须大于帐户建立的初始日期，或为Null<Datetime>()
     * @param ktype 日期的类型
     * @return 资产详情
     * @note 当datetime等于Null<Datetime>()时，与getFunds(KType)同
     */
    FundsRecord getFunds(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY);

    /**
     * 获取资产净值曲线，含借入的资产
     * @param dates 日期列表，根据该日期列表获取其对应的资产净值曲线
     * @param ktype K线类型，必须与日期列表匹配，默认KQuery::DAY
     * @return 资产净值列表
     */
    PriceList getFundsCurve(const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY);

    /**
     * 获取收益曲线，即扣除历次存入资金后的资产净值曲线
     * @param dates 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
     * @param ktype K线类型，必须与日期列表匹配，默认为KQuery::DAY
     * @return 收益曲线
     */
    PriceList getProfitCurve(const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY);

protected:
    string m_name;
    TMPtr m_tm;
    SEPtr m_se;
    AFPtr m_af;

    std::set<SYSPtr> m_running_sys_set;    // 当前仍在运行的子系统集合
    std::list<SYSPtr> m_running_sys_list;  // 当前仍在运行的子系统列表
    std::set<SYSPtr> m_all_sys_set;        // 记录所有运行过或运行中的子系统集合
    KQuery m_query;                        // 关联的查询条件
    bool m_is_ready;                       // 是否已做好运行准备

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
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        string name;
        ar& boost::serialization::make_nvp("name", name);
        m_name = UTF8ToGB(name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
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

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_PORTFOLIO_H_ */
