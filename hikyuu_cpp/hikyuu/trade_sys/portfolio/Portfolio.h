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
    Portfolio();
    Portfolio(const string& name);
    Portfolio(const TradeManagerPtr& tm,
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
    bool readyForRun();

    void rebuildOnlyTotalTM();

protected:
    string m_name;
    SEPtr  m_se;
    AFPtr  m_af;
    TMPtr  m_tm;

    //以下为临时变量
    TMPtr  m_tm_shadow;    //影子账户，用于内部协调分配资金

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
