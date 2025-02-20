/*
 * SimplePortfolio.h
 *
 *  Created on: 2016年2月21日
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
 * 资产组合
 * @ingroup SimplePortfolio
 */
class HKU_API SimplePortfolio : public Portfolio {
    PORTFOLIO_IMP(SimplePortfolio)

public:
    SimplePortfolio();
    SimplePortfolio(const TradeManagerPtr& tm, const SelectorPtr& se, const AFPtr& af);
    virtual ~SimplePortfolio();

private:
    SystemList m_dlist_sys_list;               // 因证券退市，无法执行卖出的系统（资产全部损失）
    SystemWeightList m_delay_adjust_sys_list;  // 延迟调仓卖出的系统列表
    SystemWeightList m_tmp_selected_list;
    SystemWeightList m_tmp_will_remove_sys;

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

#endif /* TRADE_SYS_PORTFOLIO_SIMPLE_H_ */
