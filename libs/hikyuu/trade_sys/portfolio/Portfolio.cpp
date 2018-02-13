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
        const SystemPtr& sys,
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
    m_sys_list.clear();
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    if (m_se) p->m_se = m_se->clone();
    if (m_af) p->m_af = m_af->clone();
    if (m_tm) p->m_tm = m_tm->clone();
    if (m_tm_shadow) p->m_tm_shadow = m_tm_shadow->clone();
    p->m_sys_list = m_sys_list;
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

    m_tm_shadow = m_tm->clone();

    //将影子账户指定给资产分配器
    m_af->setTM(m_tm_shadow);

    //为每一个圈定的系统实例设置一个初始为0的子账户，并缓存所有的系统实例
    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "SUB");
    SystemList all_sys_list = m_se->getAllSystemList();
    auto sysIter = all_sys_list.begin();
    for (; sysIter != all_sys_list.end(); ++sysIter) {
        (*sysIter)->setTM(pro_tm->clone());
        m_sys_list.push_back(*sysIter);
    }

    return true;
}

void Portfolio::run(const KQuery& query) {
    if (!readyForRun()) {
        return;
    }

    auto sys_iter = m_sys_list.begin();
    for (; sys_iter != m_sys_list.end(); ++sys_iter) {
        SystemPtr sys = *sys_iter;
        if (sys->readyForRun()) {
            KData k = sys->getStock().getKData(query);
            sys->setTO(k);
        }
    }

    SystemList delay_sys_list;
    for (sys_iter = m_sys_list.begin(); sys_iter != m_sys_list.end(); ++sys_iter) {
        if ((*sys_iter)->getParam<bool>("delay")) {
            delay_sys_list.push_back(*sys_iter);
        }
    }

    std::set<SYSPtr> cur_hold_sys_sets;

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    DatetimeList::const_iterator date_iter = datelist.begin();
    for(; date_iter != datelist.end(); ++date_iter) {
        //忽略小于账户初始建立日期的交易日
        if (*date_iter < m_tm->initDatetime()) {
            continue;
        }

        //处理延迟操作请求
        sys_iter = delay_sys_list.begin();
        for (; sys_iter != delay_sys_list.end(); ++sys_iter) {
            (*sys_iter)->_processRequest(*date_iter);

            //if ((*sys_iter)->getTM()->have((*sys_iter)->getStock())) {
            if ((*sys_iter)->getTM()->getStockNumber() != 0) {
                cur_hold_sys_sets.insert(*sys_iter);
            } else {
                cur_hold_sys_sets.erase(*sys_iter);
            }

            //同步交易记录
            TradeRecordList tr_list = (*sys_iter)->getTM()->getTradeList(*date_iter, Null<Datetime>());
            auto tr_iter = tr_list.begin();
            for (; tr_iter != tr_list.end(); ++tr_iter) {
                m_tm_shadow->addTradeRecord(*tr_iter);
            }
        }

        SystemList cur_hold_sys_list;
        auto hold_iter = cur_hold_sys_sets.begin();
        for (; hold_iter != cur_hold_sys_sets.end(); ++hold_iter) {
            cur_hold_sys_list.push_back(*hold_iter);
        }

        //计算当前时刻选择的系统实例
        SystemList selected_list = m_se->getSelectedSystemList(*date_iter);
        SystemList ac_list = m_af->getAllocatedSystemList(*date_iter,
                                           selected_list, cur_hold_sys_list);

        for (sys_iter = ac_list.begin(); sys_iter != ac_list.end(); ++sys_iter) {
            (*sys_iter)->runMoment(*date_iter);

            //同步交易记录
            TradeRecordList tr_list = (*sys_iter)->getTM()->getTradeList(*date_iter, Null<Datetime>());
            auto tr_iter = tr_list.begin();
            for (; tr_iter != tr_list.end(); ++tr_iter) {
                m_tm_shadow->addTradeRecord(*tr_iter);
            }
        }

        //同步总账户和影子账户交易记录
        TradeRecordList tr_list = m_tm_shadow->getTradeList(*date_iter, Null<Datetime>());
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
            HKU_INFO(m_tm->currentCash() << " -- " << m_tm_shadow->currentCash());
        }
    }
}


void Portfolio::runOneMoment(Datetime date) {
    if (date < m_tm->initDatetime()) {
        return;
    }

}


} /* namespace hku */
