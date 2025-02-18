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
 * 资产组合
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
    SystemList m_real_sys_list;  // 所有实际运行的子系统列表

    // 用于中间计算的临时数据
    std::unordered_set<SYSPtr> m_running_sys_set;
    SystemList m_dlist_sys_list;               // 因证券退市，无法执行卖出的系统（资产全部损失）
    SystemWeightList m_delay_adjust_sys_list;  // 延迟调仓卖出的系统列表
    SystemWeightList m_tmp_selected_list;
    SystemWeightList m_tmp_will_remove_sys;

    // 仅用于无 AF 模式，记录指派给SE的系统到内部实际系统映射
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
