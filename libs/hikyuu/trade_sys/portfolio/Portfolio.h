/*
 * Portfolio.h
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

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
    enum MODE {
        ONLY_USE_TOTAL_TM                  = 0, /**< 以总账户为原型，自动设置所有系统实例的子账户，子账户初始资金为0 */
        REBUILD_TM_AND_AUTO_CREATE_SUBTM   = 1, /**< 从各子账户重建总账户及其交易记录，未自行绑定子账户的系统实例以总账户为原型自动创建子账户 */
        REBUILD_TM_AND_IGNORE_NO_SUBTM_SYS = 2, /**< 从各子账户重建总账户及其交易记录，忽略未自行绑定子账户的系统实例 */
        ONLY_AUTO_CREATE_SUBTM             = 3,/**< 不重建总账户直接使用各系统绑定的子账户，未绑定子账户的系统实例以总账户为原型自动创建子账户 */
        ONLY_IGNORE_NO_SUBTM_SYS           = 4, /**< 不重建总账户直接使用各系统绑定的子账户，忽略未自行绑定子账户的系统实例 */
        INVLID_MODE                        = 5,
    };

public:
    Portfolio();
    Portfolio(const string& name);
    Portfolio(const TradeManagerPtr& tm,
            const SystemPtr& sys,
            const SelectorPtr& st,
            const AFPtr& af);
    virtual ~Portfolio();

    string name() const { return m_name; }
    void name(const string& name) { m_name = name; }

    void run(const KQuery& query);

    TMPtr getTM() { return m_tm; }
    SEPtr getSE() { return m_se; }
    AFPtr getAF() { return m_af; }

    void setTM(const TMPtr& tm) { m_tm = tm; }
    void setSE(const SEPtr& se) { m_se = se; }
    void setAF(const AFPtr& af) { m_af = af; }

    void reset();

    typedef shared_ptr<Portfolio> PortfolioPtr;
    PortfolioPtr clone();

private:
    void initParam();
    bool readyForRun();

    void rebuildOnlyTotalTM();
    void rebuildTMAndAutoCreateSubTM();
    void rebuildTMAndIgnore();
    void rebuildOnlyAutoCreateSubTM();
    void rebuildOnlyIngore();

    void runOneMoment(Datetime);

protected:
    string m_name;
    SEPtr  m_se;
    AFPtr  m_af;
    TMPtr  m_tm;

    SystemList m_sys_list; //临时变量，缓存从SE获取的全部系统实例，仅在调用readyForRun后有效

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        string name_str(GBToUTF8(m_name));
        ar & boost::serialization::make_nvp("name", name_str);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_se);
        ar & BOOST_SERIALIZATION_NVP(m_af);
        ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("name", name);
        m_name = UTF8ToGB(name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_se);
        ar & BOOST_SERIALIZATION_NVP(m_af);
        ar & BOOST_SERIALIZATION_NVP(m_tm);
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

HKU_API std::ostream & operator<<(std::ostream&, const Portfolio&);
HKU_API std::ostream & operator<<(std::ostream&, const PortfolioPtr&);

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_PORTFOLIO_H_ */
