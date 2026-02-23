/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#include "FactorImp.h"

namespace hku {

string FactorImp::str() const {
    std::ostringstream os;
    string strip("  \n");
    os << "FactorImp(";
    os << strip << "name: " << name() << strip << "ktype: " << ktype() << strip
       << "need_persist: " << needPersist() << strip << "create_at: " << createAt().str() << strip
       << "update_at: " << updateAt().str() << strip << "formula: " << formula() << strip
       << "brief: " << brief() << strip << "detail: " << details() << strip
       << "start_date: " << startDate() << strip << "block: " << block() << ")";
    return os.str();
}

FactorImp::FactorImp(const string& name, const Indicator& formula, const KQuery::KType& ktype,
                     const string& brief, const string& details, bool need_persist,
                     const Datetime& start_date, const Block& block)
: m_name(name),
  m_ktype(ktype),
  m_brief(brief),
  m_details(details),
  m_start_date(start_date),
  m_formula(formula),
  m_block(block),
  m_need_persist(need_persist) {}

uint64_t FactorImp::hash() const noexcept {
    return (uint64_t)this;
}

void FactorImp::save() {
    HKU_WARN("FactorImp::save() is not supported!");
}

void FactorImp::remove() {
    HKU_WARN("FactorImp::remove() is not supported!");
}

void FactorImp::load() {
    HKU_WARN("FactorImp::load() is not supported!");
}

IndicatorList FactorImp::getValues(const StockList& stocks, const KQuery& query) const {
    IndicatorList ret;
    HKU_IF_RETURN(stocks.empty(), ret);
    ret = global_parallel_for_index(0, stocks.size(), [&, this](size_t i) {
        auto k = stocks[i].getKData(query);
        return m_formula(k);
    });
    return ret;
}

}  // namespace hku