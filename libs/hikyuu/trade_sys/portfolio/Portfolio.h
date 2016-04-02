/*
 * Portfolio.h
 *
 *  Created on: 2016年2月21日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_PORTFOLIO_PORTFOLIO_H_
#define TRADE_SYS_PORTFOLIO_PORTFOLIO_H_

#include "AllocateMoneyBase.h"
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
            const SystemPtr& sys,
            const SelectorPtr& st,
            const AllocateMoneyPtr& am);
    virtual ~Portfolio();

    string name() const { return m_name; }
    void name(const string& name) { m_name = name; }

    void addStock(const Stock&);
    void addStockList(const StockList&);

    void run(const KQuery& query);

    TradeManagerPtr getTM() { return m_tm; }
    SelectorPtr getST() { return m_st; }
    SystemPtr getSYS() { return m_sys; }
    AllocateMoneyPtr getAM() { return m_am; }

    void setTM(const TradeManagerPtr& tm) { m_tm = tm; }
    void setST(const SelectorPtr& st) { m_st = st; }
    void setSYS(const SystemPtr& sys);
    void setAM(const AllocateMoneyPtr& am) { m_am = am; }

    void reset();

    typedef shared_ptr<Portfolio> PortfolioPtr;
    PortfolioPtr clone();

protected:
    string           m_name;
    SelectorPtr      m_st;
    AllocateMoneyPtr m_am;
    SystemPtr        m_sys;
    TradeManagerPtr  m_tm;

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
        ar & BOOST_SERIALIZATION_NVP(m_st);
        ar & BOOST_SERIALIZATION_NVP(m_am);
        ar & BOOST_SERIALIZATION_NVP(m_sys);
        ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("name", name);
        m_name = UTF8ToGB(name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_st);
        ar & BOOST_SERIALIZATION_NVP(m_am);
        ar & BOOST_SERIALIZATION_NVP(m_sys);
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

HKU_API std::ostream & operator<<(std::ostream&, const Portfolio&);
HKU_API std::ostream & operator<<(std::ostream&, const PortfolioPtr&);

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_PORTFOLIO_H_ */
