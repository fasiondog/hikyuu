/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#include "Factor.h"
#include "hikyuu/plugin/factor.h"
#include "hikyuu/plugin/device.h"
#include "hikyuu/StockManager.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/ALIGN.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Factor& factor) {
    os << factor.str();
    return os;
}

string Factor::str() const {
    std::ostringstream os;
    string strip("  \n");
    os << "Factor(";
    os << strip << "name: " << name() << strip << "ktype: " << ktype() << strip
       << "need_persist: " << needPersist() << strip << "create_at: " << createAt().str() << strip
       << "update_at: " << updateAt().str() << strip << "formula: " << formula().formula() << strip
       << "brief: " << brief() << strip << "detail: " << details() << strip
       << "start_date: " << startDate() << strip << "block: " << block() << ")";
    return os.str();
}

Factor::Factor() : m_data(make_shared<Factor::Data>()) {}

Factor::Factor(const string& name, const KQuery::KType& ktype)
: m_data(make_shared<Data>(name, Indicator(), ktype, "", "", false, Datetime::min(), Block())) {
    try {
        load_from_db();
    } catch (const std::exception& e) {
        HKU_ERROR("Failed to load factor from db: {}", e.what());
    }
}

Factor::Factor(const string& name, const Indicator& formula, const KQuery::KType& ktype,
               const string& brief, const string& details, bool need_persist,
               const Datetime& start_date, const Block& block)
: m_data(make_shared<Data>(name, formula, ktype, brief, details, need_persist, start_date, block)) {
}

Factor::Factor(const Factor& other) : m_data(other.m_data) {}

Factor::Factor(Factor&& other) : m_data(std::move(other.m_data)) {}

Factor& Factor::operator=(const Factor& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = other.m_data;
    return *this;
}

Factor& Factor::operator=(Factor&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = std::move(other.m_data);
    return *this;
}

IndicatorList Factor::getValues(const StockList& stocks, const KQuery& query, bool align,
                                bool fill_null, bool tovalue, bool check) const {
    if (check) {
        const auto& block = this->block();
        if (!block.empty()) {
            for (auto& stock : stocks) {
                HKU_CHECK(block.have(stock), "Stock not belong to block! {}", stock);
            }
        }
    }

    IndicatorList ret;
    HKU_IF_RETURN(stocks.empty(), ret);

    if (isValidLicense()) {
        ret = hku::getValues(*this, stocks, query, align, fill_null, tovalue);
        return ret;
    }

    if (align) {
        DatetimeList dates = StockManager::instance().getTradingCalendar(query);
        HKU_IF_RETURN(dates.empty(), ret);
        auto null_ind = PRICELIST(PriceList(dates.size(), Null<price_t>()), dates);
        ret = global_parallel_for_index(0, stocks.size(), [&, tovalue, this](size_t i) {
            Indicator cur_ind;
            auto k = stocks[i].getKData(query);
            HKU_IF_RETURN(k.empty(), null_ind);
            return tovalue ? ALIGN(formula(), dates, fill_null)(k).getResult(0)
                           : ALIGN(formula(), dates, fill_null)(k);
        });

    } else {
        ret = global_parallel_for_index(0, stocks.size(), [&, tovalue, this](size_t i) {
            auto k = stocks[i].getKData(query);
            return tovalue ? formula()(k).getResult(0) : formula()(k);
        });
    }
    return ret;
}

IndicatorList Factor::getAllValues(const KQuery& query, bool align, bool fill_null, bool tovalue) {
    StockList stocks =
      block().empty() ? StockManager::instance().getStockList() : block().getStockList();
    return getValues(stocks, query, align, fill_null, tovalue, false);
}

void Factor::save_to_db() {
    saveFactor(*this);
}

void Factor::remove_from_db() {
    removeFactor(name(), ktype());
}

void Factor::load_from_db() {
    Factor tmp = getFactor(name(), ktype());
    m_data = std::move(tmp.m_data);
}

}  // namespace hku