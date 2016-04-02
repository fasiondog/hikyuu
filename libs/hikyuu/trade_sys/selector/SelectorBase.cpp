/*
 * SelectorBase.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: Administrator
 */

#include <hikyuu/trade_sys/selector/SelectorBase.h>

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const SelectorBase& st) {
    os << "Selector(" << st.name() << ", " << st.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const SelectorPtr& st) {
    if (st) {
        os << *st;
    } else {
        os << "Selector(NULL)";
    }

    return os;
}

SelectorBase::SelectorBase() {

}


SelectorBase::~SelectorBase() {

}

void SelectorBase::clearStockList() {
    m_stock_list.clear();
}

void SelectorBase::reset() {
    _reset();
}

SelectorPtr SelectorBase::clone() {
    SelectorPtr p = _clone();
    if (p) {
        p->m_params = m_params;
        p->m_stock_list = m_stock_list;
    }
    return p;
}


void SelectorBase::addStock(const Stock& stock) {
    if (stock.isNull()) {
        HKU_WARN("Try add Null stock, will be discard! "
                "[SelectorBase::addStock]");
        return;
    }

    bool find = false;
    for (auto iter = m_stock_list.begin(); iter != m_stock_list.end(); ++iter) {
        if (stock == *iter) {
            find = true;
            break;
        }
    }

    if (!find) {
        m_stock_list.push_back(stock);
    }
}


void SelectorBase::
addStockList(const StockList& stk_list) {
    m_stock_list.reserve(m_stock_list.size() + stk_list.size());
    for (auto iter = stk_list.begin(); iter != stk_list.end(); ++iter) {
        addStock(*iter);
    }
}


StockList SelectorBase::getSelectedStock(Datetime date) {
    return m_stock_list;
}

} /* namespace hku */
