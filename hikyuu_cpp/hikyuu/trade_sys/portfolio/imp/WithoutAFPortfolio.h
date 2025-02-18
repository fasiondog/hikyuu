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
 * 无资金分配算法的资产组合
 * @ingroup WithoutAFPortfolio
 */
class HKU_API WithoutAFPortfolio : public Portfolio {
    PORTFOLIO_IMP(WithoutAFPortfolio)

public:
    WithoutAFPortfolio();
    WithoutAFPortfolio(const TradeManagerPtr& tm, const SelectorPtr& se);
    virtual ~WithoutAFPortfolio();

private:
    void initParam();
    void _runMomentWithoutAFNotForceSell(const Datetime& date, const Datetime& nextCycle,
                                         bool adjust);
    void _runMomentWithoutAFForceSell(const Datetime& date, const Datetime& nextCycle, bool adjust);

private:
    SystemList m_force_sell_sys_list;  // 强制卖出的系统列表
    list<SYSPtr> m_running_sys_list;   // 当前运行中的系统列表，需要依次执行
    SystemList m_selected_list;        // 本轮周期内选中的系统列表

    // 记录指派给SE的系统到内部实际系统映射
    unordered_map<SYSPtr, SYSPtr> m_se_sys_to_pf_sys_dict;

//============================================
// 序列化支持
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
