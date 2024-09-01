/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include "StrategyContext.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const StrategyContext& context) {
    os << context.str();
    return os;
}

string StrategyContext::str() const {
    std::stringstream os;
    os << "StrategyContext{\n"
       << "  start datetime: " << m_startDatetime << ",\n"
       << "  stock code list: [";
    size_t len = m_stockCodeList.size();
    if (len > 5) {
        len = 5;
    }
    for (size_t i = 0; i < len; i++) {
        os << "\"" << m_stockCodeList[i] << "\", ";
    }
    if (m_stockCodeList.size() >= 5) {
        os << "...";
    }
    os << "],\n"
       << "  ktype list: [";
    for (size_t i = 0, total = m_ktypeList.size(); i < total; i++) {
        os << "\"" << m_ktypeList[i] << "\", ";
    }
    os << "],\n"
       << "  default load: [";
    for (size_t i = 0, total = m_mustLoad.size(); i < total; i++) {
        os << "\"" << m_mustLoad[i] << "\", ";
    }
    os << "],\n"
       << "}";
    return os.str();
}

StrategyContext::StrategyContext(const vector<string>& stockCodeList) {
    _removeDuplicateCode(stockCodeList);
}

StrategyContext::StrategyContext(const vector<string>& stockCodeList,
                                 const vector<KQuery::KType>& ktypeList) {
    _removeDuplicateCode(stockCodeList);
    _checkAndRemoveDuplicateKType(ktypeList);
}

void StrategyContext::_removeDuplicateCode(const vector<string>& stockCodeList) {
    m_stockCodeList.clear();
    m_stockCodeList.reserve(stockCodeList.size());
    std::set<string> code_set;
    for (const auto& code : stockCodeList) {
        if (code_set.find(code) == code_set.end()) {
            m_stockCodeList.push_back(code);
        } else {
            code_set.insert(code);
        }
    }
}

void StrategyContext::_checkAndRemoveDuplicateKType(const vector<KQuery::KType>& ktypeList) {
    m_ktypeList.clear();
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
    vector<string> ret;
    std::set<string> code_set;
    for (const auto& code : m_mustLoad) {
        ret.push_back(code);
        code_set.insert(code);
    }
    for (const auto& code : m_stockCodeList) {
        if (code_set.find(code) == code_set.end()) {
            ret.push_back(code);
        }
    }
    return ret;
}

}  // namespace hku