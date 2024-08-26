/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include "StrategyContext.h"

namespace hku {

void StrategyContext::setStockCodeList(const vector<string>& stockList) {
    m_stockCodeList.resize(stockList.size());
    std::copy(stockList.begin(), stockList.end(), m_stockCodeList.begin());
}

void StrategyContext::setKTypeList(const vector<KQuery::KType>& ktypeList) {
    m_ktypeList.resize(ktypeList.size());
    std::transform(ktypeList.begin(), ktypeList.end(), m_ktypeList.begin(),
                   [](KQuery::KType ktype) {
                       to_upper(ktype);
                       return ktype;
                   });

    // 对 ktype 按时间长度进行升序排序
    std::sort(m_ktypeList.begin(), m_ktypeList.end(),
              [](const KQuery::KType& a, const KQuery::KType& b) {
                  return KQuery::getKTypeInMin(a) < KQuery::getKTypeInMin(b);
              });
}

bool StrategyContext::isAll() const noexcept {
    return std::find_if(m_stockCodeList.begin(), m_stockCodeList.end(), [](string val) {
               to_upper(val);
               return val == "ALL";
           }) != m_stockCodeList.end();
}

bool StrategyContext::isValid() const noexcept {
    return m_stockCodeList.empty() || m_ktypeList.empty();
}

}  // namespace hku