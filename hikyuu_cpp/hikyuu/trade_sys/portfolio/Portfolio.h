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
class HKU_API Portfolio : public enable_shared_from_this<Portfolio> {
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

    /**
     * @brief 运行资产组合
     * @note
     * 由于各个组件可能存在参数变化的情况，无法自动感知判断是否需要重新计算，此时需要手工指定强制计算
     * @param query 查询条件
     * @param force 是否强制重计算
     */
    void run(const KQuery& query, bool force = false);

    void runMoment(const Datetime& datetime);

    void setQuery(const KQuery& query) {
        if (m_query != query) {
            m_query = query;
            m_need_calculate = true;
        }
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
        if (m_tm != tm) {
            m_tm = tm;
            m_need_calculate = true;
        }
    }

    void setSE(const SEPtr& se) {
        if (m_se != se) {
            m_se = se;
            m_need_calculate = true;
        }
    }

    void setAF(const AFPtr& af) {
        if (m_af != af) {
            m_af = af;
            m_need_calculate = true;
        }
    }

    void reset();

    typedef shared_ptr<Portfolio> PortfolioPtr;
    PortfolioPtr clone();

    const SystemList& getProtoSystemList() const {
        return m_pro_sys_list;
    }

    const SystemList& getRealSystemList() const {
        return m_real_sys_list;
    }

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
     * 获取从账户建立日期到系统当前日期的资产净值曲线（按自然日）
     * @return 资产净值列表
     */
    PriceList getFundsCurve();

    /**
     * 获取收益曲线，即扣除历次存入资金后的资产净值曲线
     * @param dates 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
     * @param ktype K线类型，必须与日期列表匹配，默认为KQuery::DAY
     * @return 收益曲线
     */
    PriceList getProfitCurve(const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY);

    /**
     * 获取获取从账户建立日期到系统当前日期的收益曲线
     * @return 收益曲线
     */
    PriceList getProfitCurve();

private:
    void _runMomentOnOpen(const Datetime& datetime);
    void _runMomentOnClose(const Datetime& datetime);

protected:
    string m_name;
    TMPtr m_tm;
    TMPtr m_shadow_tm;
    SEPtr m_se;
    AFPtr m_af;

    SystemList m_pro_sys_list;
    SystemList m_real_sys_list;
    std::set<SYSPtr> m_running_sys_set;    // 当前仍在运行的子系统集合
    std::list<SYSPtr> m_running_sys_list;  // 当前仍在运行的子系统列表
    KQuery m_query;                        // 关联的查询条件
    bool m_is_ready;                       // 是否已做好运行准备
    bool m_need_calculate;                 // 是否需要计算标志

    // 用于中间计算的临时数据
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
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
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
