/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include <boost/bind.hpp>
#include "../../trade_manage/crt/crtTM.h"

#include "Portfolio.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const Portfolio& pf) {
    os << "Portfolio(" << pf.name() << ", " << pf.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const PortfolioPtr& pf) {
    if (pf) {
        os << *pf;
    } else {
        os << "Portfolio(NULL)";
    }

    return os;
}

Portfolio::Portfolio(): m_name("Portfolio") {

}

Portfolio::Portfolio(const string& name) : m_name(name) {

}

Portfolio::Portfolio(const TradeManagerPtr& tm,
        const SelectorPtr& se,
        const AFPtr& af)
: m_name("Portfolio"), m_se(se), m_tm(tm), m_af(af) {

}

Portfolio::~Portfolio() {

}

void Portfolio::reset() {
    if (m_tm) m_tm->reset();
    if (m_se) m_se->reset();
    if (m_af) m_af->reset();
    if (m_tm_shadow) m_tm_shadow->reset();
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    if (m_se) p->m_se = m_se->clone();
    if (m_af) p->m_af = m_af->clone();
    if (m_tm) p->m_tm = m_tm->clone();
    if (m_tm_shadow) p->m_tm_shadow = m_tm_shadow->clone();
    return p;
}

bool Portfolio::readyForRun() {
    string func_name(" [Portfolio::readyForRun]");
    if (!m_se) {
        HKU_WARN("m_se is null!" << func_name);
        return false;
    }

    if (!m_tm) {
        HKU_WARN("m_tm is null!" << func_name);
        return false;
    }

    if (!m_af) {
        HKU_WARN("m_am is null!" << func_name);
        return false;
    }

    reset();

    //将影子账户指定给资产分配器
    m_tm_shadow = m_tm->clone();
    m_af->setTM(m_tm_shadow);

    return true;
}

void Portfolio::run(const KQuery& query) {
    if (!readyForRun()) {
        return;
    }

    m_af->setQuery(query);

    SystemList all_sys_list = m_se->getAllSystemList();
    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "SUB");
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {

        //为每一个系统实例分配子账户
        SystemPtr& sys = *sys_iter;
        sys->setTM(pro_tm->clone());

        if (sys->readyForRun()) {
            KData k = sys->getStock().getKData(query);
            sys->setTO(k);
        }
    }

    SystemList       cur_selected_list;  //当前选中系统列表
    std::set<SYSPtr> cur_selected_sets;  //当前选中系统集合，方便计算使用
    SystemList cur_allocated_list; //当前分配了资金的系统
    SystemList cur_hold_sys_list;  //当前时刻有持仓的系统，每个时刻重新收集

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    DatetimeList::const_iterator date_iter = datelist.begin();
    for(; date_iter != datelist.end(); ++date_iter) {
        const Datetime& cur_date = *date_iter;

        //忽略小于账户初始建立日期的交易日
        if (cur_date < m_tm->initDatetime()) {
            continue;
        }

        cur_hold_sys_list.clear();

        //----------------------------------------------------------
        //如果当前时刻选择标的发生变化（此时也一定是资金调整分配的时刻）
        //----------------------------------------------------------
        bool selected_changed = m_se->changed(cur_date);
        if (selected_changed) {

            //重新计算当前时刻选择的系统实例
            cur_selected_list = m_se->getSelectedSystemList(cur_date);

            //构建当前时刻选择的系统实例集合，便于后续计算
            cur_selected_sets.clear();
            sys_iter = cur_selected_list.begin();
            for (; sys_iter != cur_selected_list.end(); ++sys_iter) {
                cur_selected_sets.insert(*sys_iter);
            }

        }

        //----------------------------------------------------------
        //查找当前已分配资金的系统，如果不在当前选中的系统范围内，则回收期没有持仓的系统资金
        //----------------------------------------------------------
        sys_iter = cur_allocated_list.begin();
        for (; sys_iter != cur_allocated_list.end(); ++sys_iter) {
            SYSPtr& sys = *sys_iter;
            TMPtr tm = sys->getTM();

            if (tm->getStockNumber() != 0) {
                //收集当前仍有持仓的系统
                cur_hold_sys_list.push_back(sys);

            } else {
                //如果该系统已没有持仓，且不在当前的选中系统范围内，则回收分配的资金
                if (cur_selected_sets.find(sys) == cur_selected_sets.end()) {
                    price_t cash = tm->currentCash();
                    if (cash > 0) {
                        //要先存入影子账户再取出子账户资金，否则后续交易记录同步会错误
                        m_tm_shadow->checkin(cur_date, cash);
                        tm->checkout(cur_date, cash);
                    }
                }
            }
        }

        //----------------------------------------------------------
        //如果选择列表更新或调整资金时刻，则调整资金分配
        //----------------------------------------------------------
        if (selected_changed || m_af->changed(cur_date)) {
            cur_allocated_list = m_af->getAllocatedSystemList(cur_date,
                    cur_selected_list, cur_hold_sys_list);
        }

        //----------------------------------------------------------
        // 运行当前分配了资金的系统
        //----------------------------------------------------------
        sys_iter = cur_allocated_list.begin();
        for (; sys_iter != cur_allocated_list.end(); ++sys_iter) {
            SYSPtr& sys = *sys_iter;
            sys->runMoment(cur_date);

            //同步交易记录
            TradeRecordList tr_list = sys->getTM()->getTradeList(cur_date, Null<Datetime>());
            auto tr_iter = tr_list.begin();
            for (; tr_iter != tr_list.end(); ++tr_iter) {
                m_tm_shadow->addTradeRecord(*tr_iter);
            }
        }

        //同步总账户和影子账户交易记录
        TradeRecordList tr_list = m_tm_shadow->getTradeList(cur_date, Null<Datetime>());
        auto tr_iter = tr_list.begin();
        for (; tr_iter != tr_list.end(); ++tr_iter) {
            if (tr_iter->business == BUSINESS_CHECKIN
             || tr_iter->business == BUSINESS_CHECKOUT) {
                continue;
            }

            m_tm->addTradeRecord(*tr_iter);
        }

        if (m_tm->currentCash() != m_tm_shadow->currentCash()) {
            HKU_INFO("m_tm->currentCash() != m_tm_shadow->currentCash()");
            HKU_INFO(m_tm->currentCash() << " == " << m_tm_shadow->currentCash());
        }

    } // for datelist
}



} /* namespace hku */
