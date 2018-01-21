/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/portfolio/Portfolio.h>

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
: m_se(se), m_tm(tm), m_sys(sys), m_name("Portfolio") {

}

Portfolio::~Portfolio() {

}

void Portfolio::reset() {
    if (m_tm) m_tm->reset();
    if (m_se) m_se->reset();
    if (m_sys) m_sys->reset(false, true, true, true);
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_se = m_se;
    p->m_tm = m_tm;
    p->m_sys = m_sys;
    return p;
}

void Portfolio::addStock(const Stock& stock) {
    if (m_se) {
        m_se->addStock(stock);
    } else {
        HKU_WARN("m_se is Null! [Portfolio::addStock]");
    }
}

void Portfolio::addStockList(const StockList& stock_list) {
    if (m_se) {
        m_se->addStockList(stock_list);
    } else {
        HKU_WARN("m_se is Null! [Portfolio::addStockList]");
    }
}


void Portfolio::run(const KQuery& query) {
    if (!m_se) {
        HKU_WARN("m_se is null [Portfolio::run]");
        return;
    }

    if (!m_tm) {
        HKU_WARN("m_tm is null [Portfolio::run]");
        return;
    }

    if (!m_sys) {
        HKU_WARN("m_sys is null [Portfolio::run]");
        return;
    }

    m_sys->setTM(m_tm);
    reset();

    map<string, SystemPtr> stock_map_buffer; //缓存
    map<string, SystemPtr>::iterator stock_map_iter;

    std::set<SystemPtr> processed_sys;

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    DatetimeList::const_iterator date_iter = datelist.begin();
    for(; date_iter != datelist.end(); ++date_iter) {
        //忽略小于账户初始建立日期的交易日
        if (*date_iter < m_tm->initDatetime()) {
            continue;
        }

        processed_sys.clear();

        //优先处理处理有延迟操作请求的系统策略
        stock_map_iter = stock_map_buffer.begin();
        for (; stock_map_iter != stock_map_buffer.end(); ++stock_map_iter) {
            if (stock_map_iter->second->haveDelayRequest()) {
                stock_map_iter->second->runMoment(*date_iter);
                processed_sys.insert(stock_map_iter->second);
            }
        }

        //根据选股规则，计算当前时刻选择的股票列表
        StockList stk_list = m_se->getSelectedStockList(*date_iter);

        StockList::const_iterator stk_iter = stk_list.begin();
        for (; stk_iter != stk_list.end(); ++stk_iter) {
            SystemPtr sys;

            //是否已经在缓存中
            stock_map_iter = stock_map_buffer.find(stk_iter->market_code());
            if (stock_map_iter != stock_map_buffer.end()) {
                sys = stock_map_iter->second;

                //如果已在之前处理过延迟请求时运行过，则忽略
                if (processed_sys.count(sys))
                    continue;

            } else {
                sys = m_sys->clone(false, false, false, false);
                if (sys->readyForRun()) {
                    KData k = stk_iter->getKData(query);
                    sys->setTO(k);
                    stock_map_buffer[stk_iter->market_code()] = sys;
                }
            }

            sys->runMoment(*date_iter);
        }
    }
}


} /* namespace hku */
