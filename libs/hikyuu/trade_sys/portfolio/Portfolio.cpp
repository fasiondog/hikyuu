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

    SystemList delay_sys_list;  //存储延迟模式的系统列表

    SystemList all_sys_list = m_se->getAllSystemList();
    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "SUB");
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {

        //为每一个系统实例分配子账户
        SystemPtr& sys = *sys_iter;
        sys->setTM(pro_tm->clone());

        if (sys->readyForRun()) {
            if (sys->getParam<bool>("delay")) {
                delay_sys_list.push_back(sys);
            }

            KData k = sys->getStock().getKData(query);
            sys->setTO(k);
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
            SYSPtr& sys = *sys_iter;
            if (!sys->haveDelayRequest()) {
                continue;
            }

            Stock stk = sys->getStock();
            if (stk.isNull()) {
                continue;
            }

            KRecord kr = stk.getKRecordByDate(*date_iter);
            //sys->_processRequest(kr);

            if (sys->getTM()->have(stk)) {
            //if ((*sys_iter)->getTM()->getStockNumber() != 0) {
                cur_hold_sys_sets.insert(sys);
            } else {
                cur_hold_sys_sets.erase(sys);
            }

            //同步交易记录
            TradeRecordList tr_list = sys->getTM()->getTradeList(*date_iter, Null<Datetime>());
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


        std::cout << "=====================" << std::endl;
        for (sys_iter = ac_list.begin(); sys_iter != ac_list.end(); ++sys_iter) {
            SYSPtr& sys = *sys_iter;
            sys->runMoment(*date_iter);

            std::cout << *date_iter << ": " << sys->getStock() << std::endl;

            if (sys->getTM()->have(sys->getStock())) {
                cur_hold_sys_sets.insert(sys);
            } else {
                cur_hold_sys_sets.erase(sys);
            }

            //同步交易记录
            TradeRecordList tr_list = sys->getTM()->getTradeList(*date_iter, Null<Datetime>());
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



} /* namespace hku */
