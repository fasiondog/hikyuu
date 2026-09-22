/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_WITHOUTAF_H_
#define TRADE_SYS_PORTFOLIO_WITHOUTAF_H_

#include "hikyuu/trade_sys/portfolio/Portfolio.h"

namespace hku {

/*
 * Portfolio without a fund allocation algorithm
 * @ingroup WithoutAFPortfolio
 */
class HKU_API WithoutAFPortfolio : public Portfolio {
    PORTFOLIO_IMP(WithoutAFPortfolio)

public:
    WithoutAFPortfolio();
    WithoutAFPortfolio(const TradeManagerPtr& tm, const SelectorPtr& se);
    virtual ~WithoutAFPortfolio();

    virtual json lastSuggestion() const override;

private:
    void initParam();
    void _runMomentWithoutAFNotForceSell(const Datetime& date, const Datetime& nextCycle,
                                         bool adjust);
    void _runMomentWithoutAFForceSell(const Datetime& date, const Datetime& nextCycle, bool adjust);

private:
    SystemList m_force_sell_sys_list;  // System list of the forced sells
    list<SYSPtr> m_running_sys_list;   // List of the currently running systems, they need to be
                                       // executed in turn
    SystemList m_selected_list;        // System list selected in the current cycle

    // Records the mapping from the systems assigned to SE to the internal actual systems
    unordered_map<SYSPtr, SYSPtr> m_se_sys_to_pf_sys_dict;

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

#endif /* TRADE_SYS_PORTFOLIO_WITHOUTAF_H_ */
