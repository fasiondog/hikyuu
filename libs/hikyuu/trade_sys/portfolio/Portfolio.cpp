/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include <boost/bind.hpp>
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
        const SelectorPtr& se)
: m_se(se), m_tm(tm), m_name("Portfolio") {

}

Portfolio::~Portfolio() {

}

void Portfolio::reset() {
    if (m_tm) m_tm->reset();
    if (m_se) m_se->reset();
    if (m_af) m_af->reset();
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_se = m_se;
    p->m_af = m_af;
    p->m_tm = m_tm;
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

    //--------------------------------------------
    // 从系统策略子账户重建总账户
    //--------------------------------------------
    Datetime min_init_date = m_tm->initDatetime();
    Datetime max_last_date = m_tm->lastDatetime();
    price_t init_cash = 0.0;
    TradeRecordList all_tr_list;

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {
        TMPtr tm = (*sys_iter)->getTM();
        if (tm) {
            if (tm->initDatetime() < min_init_date) {
                min_init_date = tm->initDatetime();
            }
            if (tm->initDatetime() == min_init_date) {
                init_cash += tm->initCash();
            }
            if (tm->lastDatetime() > max_last_date) {
                max_last_date = tm->lastDatetime();
            }

            TradeRecordList tr_list = tm->getTradeList();
            all_tr_list.insert(all_tr_list.end(), tr_list.begin(), tr_list.end());
        }
    }

    sort(all_tr_list.begin(), all_tr_list.end(),
         boost::bind(std::less<Datetime>(),
                     boost::bind(&TradeRecord::datetime, _1),
                     boost::bind(&TradeRecord::datetime, _2)));

    m_tm->addTradeRecord(TradeRecord(Null<Stock>(), min_init_date,
            BUSINESS_INIT, init_cash, init_cash, 0.0, 0,
            CostRecord(), 0.0,  init_cash, PART_INVALID));
    auto tr_iter = all_tr_list.begin();
    for (; tr_iter != all_tr_list.end(); ++tr_iter) {
        if (tr_iter->business == BUSINESS_INIT) {
            if (tr_iter->datetime != min_init_date) {
                TradeRecord new_tr(*tr_iter);
                new_tr.business = BUSINESS_CHECKIN;
                m_tm->addTradeRecord(new_tr);
            }
        } else {
            m_tm->addTradeRecord(*tr_iter);
        }
    }

    return true;
}

void Portfolio::run(const KQuery& query) {
    if (!readyForRun()) {
        return;
    }

    //reset();

    //m_af->setTM(m_tm);

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {
        SystemPtr sys = *sys_iter;

        sys->setTM(m_tm);
        if (sys->readyForRun()) {
            KData k = sys->getStock().getKData(query);
            sys->setTO(k);
        }
    }

    SystemList cur_hold_sys_list;

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    DatetimeList::const_iterator date_iter = datelist.begin();
    for(; date_iter != datelist.end(); ++date_iter) {
        //忽略小于账户初始建立日期的交易日
        if (*date_iter < m_tm->initDatetime()) {
            continue;
        }

        //处理延迟操作请求
        sys_iter = all_sys_list.begin();
        for (; sys_iter != all_sys_list.end(); ++sys_iter) {
            (*sys_iter)->_processRequest(*date_iter);
        }

        //计算当前时刻选择的股票列表
        SystemList selected_list = m_se->getSelectedSystemList(*date_iter);
        SystemWeightList sw_list = m_af->getAllocateMoney(selected_list);
        auto sw_iter = sw_list.begin();
        for (; sw_iter != sw_list.end(); ++sw_iter) {
            sw_iter->getSYS()->runMoment(*date_iter);
        }

        /*SystemList selected_sys_list = m_se->getSelectedSystemList(*date_iter);
        sys_iter = selected_sys_list.begin();
        for (; sys_iter != selected_sys_list.end(); ++sys_iter) {
            (*sys_iter)->runMoment(*date_iter);
        }*/
    }
}


void Portfolio::runOneMoment(Datetime date) {
    if (date < m_tm->initDatetime()) {
        return;
    }

}


} /* namespace hku */
