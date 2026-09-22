/*
 * SimplePortfolio.h
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#pragma once

#ifndef TRADE_SYS_PORTFOLIO_SIMPLE_H_
#define TRADE_SYS_PORTFOLIO_SIMPLE_H_

#include "hikyuu/trade_sys/allocatefunds/AllocateFundsBase.h"
#include "hikyuu/trade_sys/selector/SelectorBase.h"
#include "hikyuu/trade_sys/portfolio/Portfolio.h"

namespace hku {

/*
 * Portfolio
 * @ingroup SimplePortfolio
 */
class HKU_API SimplePortfolio : public Portfolio {
    PORTFOLIO_IMP(SimplePortfolio)

public:
    SimplePortfolio();
    SimplePortfolio(const TradeManagerPtr& tm, const SelectorPtr& se, const AFPtr& af);
    virtual ~SimplePortfolio();

    virtual json lastSuggestion() const override;

private:
    SystemList m_dlist_sys_list;               // The systems that cannot execute a sell because the
                                               // security is delisted (the whole assets are lost)
    SystemWeightList m_delay_adjust_sys_list;  // System list of the delayed rebalancing sells
    SystemWeightList m_tmp_selected_list;
    SystemWeightList m_tmp_will_remove_sys;

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(Portfolio);
    }

#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_SIMPLE_H_ */
