/*
 * RateTable.cpp
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#include "RateTable.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/arithmetic.h"

namespace hku {

namespace {
/** Whether every non-blank character of the key is a digit, i.e. the key is a security type */
bool isDigitKey(const string& key) {
    bool has_digit = false;
    for (char c : key) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            has_digit = true;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c)))
            continue;
        return false;
    }
    return has_digit;
}
}  // namespace

void RateTable::parse(const string& src, const string& tag, bool allow_zero, bool allow_one,
                      bool allow_above_one) {
    if (src == m_src)
        return;

    const string range = allow_above_one
                           ? string(allow_zero ? "[" : "(") + "0, +inf)"
                           : string(allow_zero ? "[" : "(") + "0, 1" + (allow_one ? "]" : ")");
    std::unordered_map<uint32_t, double> type_rates;
    std::unordered_map<string, double> market_rates;

    size_t pos = 0;
    while (pos < src.size()) {
        size_t end = src.find_first_of(",; \t\r\n", pos);
        end = end == string::npos ? src.size() : end;
        string item = src.substr(pos, end - pos);
        pos = end + 1;
        trim(item);
        if (item.empty())
            continue;

        size_t sep = item.find(':');
        HKU_CHECK(sep != string::npos && sep != 0 && sep + 1 < item.size(),
                  "{}: invalid rate item \"{}\", expected \"key:ratio\"!", tag, item);

        string key = item.substr(0, sep);
        string value = item.substr(sep + 1);
        trim(key);
        trim(value);

        double rate = 0.0;
        size_t used = 0;
        try {
            rate = std::stod(value, &used);
        } catch (...) {
            HKU_THROW("{}: invalid ratio \"{}\" of key \"{}\"!", tag, value, key);
        }
        HKU_CHECK(used == value.size(), "{}: invalid ratio \"{}\" of key \"{}\"!", tag, value, key);
        HKU_CHECK(std::isfinite(rate) && (allow_zero ? rate >= 0.0 : rate > 0.0) &&
                    (allow_above_one || (allow_one ? rate <= 1.0 : rate < 1.0)),
                  "{}: ratio({}) of key \"{}\" must be in {}!", tag, rate, key, range);

        if (isDigitKey(key)) {
            uint32_t type = 0;
            try {
                type = static_cast<uint32_t>(std::stoul(key));
            } catch (...) {
                HKU_THROW("{}: invalid type key \"{}\"!", tag, key);
            }
            if (type_rates.count(type) > 0)
                HKU_WARN("{}: duplicated type key \"{}\", the last one wins!", tag, key);
            type_rates[type] = rate;
        } else {
            to_upper(key);
            if (market_rates.count(key) > 0)
                HKU_WARN("{}: duplicated market key \"{}\", the last one wins!", tag, key);
            market_rates[key] = rate;
        }
    }

    // Commit only when the whole expression was accepted
    m_type_rates.swap(type_rates);
    m_market_rates.swap(market_rates);
    m_src = src;
}

double RateTable::lookup(const Stock& stock, double fallback) const {
    auto type_iter = m_type_rates.find(stock.type());
    if (type_iter != m_type_rates.end())
        return type_iter->second;

    string market(stock.market());
    to_upper(market);
    auto market_iter = m_market_rates.find(market);
    if (market_iter != m_market_rates.end())
        return market_iter->second;

    return fallback;
}

bool RateTable::has(const Stock& stock) const {
    if (m_type_rates.count(stock.type()) > 0)
        return true;

    string market(stock.market());
    to_upper(market);
    return m_market_rates.count(market) > 0;
}

double RateTable::minRate(double fallback) const {
    double result = fallback;
    for (const auto& kv : m_type_rates)
        result = std::min(result, kv.second);
    for (const auto& kv : m_market_rates)
        result = std::min(result, kv.second);
    return result;
}

} /* namespace hku */
