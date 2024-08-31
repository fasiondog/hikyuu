/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include "StrategyContext.h"

namespace hku {

StrategyContext::StrategyContext(const vector<string>& stockCodeList) {
    _removeDuplicateCode(stockCodeList);
}

StrategyContext::StrategyContext(const vector<string>& stockCodeList,
                                 const vector<KQuery::KType>& ktypeList) {
    _removeDuplicateCode(stockCodeList);
    _checkAndRemoveDuplicateKType(ktypeList);
}

StrategyContext::StrategyContext(StrategyContext&& rv)
: m_startDatetime(std::move(rv.m_startDatetime)),
  m_mustLoad(std::move(rv.m_mustLoad)),
  m_stockCodeList(std::move(rv.m_stockCodeList)),
  m_ktypeList(std::move(m_ktypeList)) {}

StrategyContext& StrategyContext::operator=(const StrategyContext& rv) {
    if (this != &rv) {
        m_startDatetime = rv.m_startDatetime;
        m_mustLoad = rv.m_mustLoad;
        m_stockCodeList = rv.m_stockCodeList;
        m_ktypeList = rv.m_ktypeList;
    }
    return *this;
}

StrategyContext& StrategyContext::operator=(StrategyContext&& rv) {
    if (this != &rv) {
        m_startDatetime = std::move(rv.m_startDatetime);
        m_mustLoad = std::move(rv.m_mustLoad);
        m_stockCodeList = std::move(rv.m_stockCodeList);
        m_ktypeList = std::move(rv.m_ktypeList);
    }
    return *this;
}

void StrategyContext::_removeDuplicateCode(const vector<string>& stockCodeList) {
    m_stockCodeList.reserve(stockCodeList.size());
    std::set<string> code_set;
    for (const auto& code : m_mustLoad) {
        code_set.insert(code);
    }
    for (const auto& code : stockCodeList) {
        if (code_set.find(code) == code_set.end()) {
            m_stockCodeList.push_back(code);
        } else {
            code_set.insert(code);
        }
    }
}

void StrategyContext::_checkAndRemoveDuplicateKType(const vector<KQuery::KType>& ktypeList) {
    m_ktypeList.reserve(ktypeList.size());
    std::set<KQuery::KType> ktype_set;
    for (const auto& ktype : ktypeList) {
        HKU_CHECK(KQuery::isKType(ktype), "Invalid ktype: {}", ktype);
        if (ktype_set.find(ktype) == ktype_set.end()) {
            m_ktypeList.push_back(ktype);
        } else {
            ktype_set.insert(ktype);
        }
    }
}

bool StrategyContext::isAll() const noexcept {
    return std::find_if(m_stockCodeList.begin(), m_stockCodeList.end(), [](string val) {
               to_upper(val);
               return val == "ALL";
           }) != m_stockCodeList.end();
}

vector<string> StrategyContext::getAllNeedLoadStockCodeList() const noexcept {
    vector<string> ret{m_stockCodeList};
    for (const auto& code : m_mustLoad) {
        ret.push_back(code);
    }
    return ret;
}

}  // namespace hku