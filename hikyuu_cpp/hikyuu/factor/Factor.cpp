/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#include "Factor.h"
#include "hikyuu/plugin/factor.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Factor& factor) {
    os << factor.str();
    return os;
}

string Factor::str() const {
    return m_imp->str();
}

shared_ptr<FactorImp> Factor::ms_null_factor_imp{make_shared<FactorImp>()};

Factor::Factor() : m_imp(ms_null_factor_imp) {}

Factor::Factor(const string& name, const KQuery::KType& ktype)
: m_imp(createFactorImp(name, Indicator(), ktype, "", "", false, Datetime::min(), Block())) {}

Factor::Factor(const string& name, const Indicator& formula, const KQuery::KType& ktype,
               const string& brief, const string& details, bool need_persist,
               const Datetime& start_date, const Block& block)
: m_imp(createFactorImp(name, formula, ktype, brief, details, need_persist, start_date, block)) {}

Factor::Factor(const Factor& other) {
    m_imp = other.m_imp;
}

Factor::Factor(Factor&& other) {
    m_imp = std::move(other.m_imp);
    other.m_imp = ms_null_factor_imp;
}

Factor& Factor::operator=(const Factor& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_imp = other.m_imp;
    return *this;
}

Factor& Factor::operator=(Factor&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_imp = std::move(other.m_imp);
    other.m_imp = ms_null_factor_imp;
    return *this;
}

IndicatorList Factor::getValues(const StockList& stocks, const KQuery& query, bool check) const {
    if (check) {
        const auto& block = this->block();
        if (!block.empty()) {
            for (auto& stock : stocks) {
                HKU_CHECK(block.have(stock), "Stock not belong to block! {}", stock);
            }
        }
    }
    return m_imp->getValues(stocks, query);
}

IndicatorList Factor::getAllValues(const KQuery& query) {
    StockList stocks =
      block().empty() ? StockManager::instance().getStockList() : block().getStockList();
    return m_imp->getValues(stocks, query);
}

}  // namespace hku