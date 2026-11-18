/*
 * RateTable.h
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_RATE_TABLE_H_
#define TRADE_MANAGE_RATE_TABLE_H_

#include <cstdint>
#include <unordered_map>
#include "../Stock.h"

namespace hku {

/**
 * A "key:ratio" lookup table shared by the margin and cost models
 * @details Items are separated by ",", ";" or blanks, e.g. "12:0.10, SHF:0.12". An all-digit key
 *          is a security type; any other is an upper-cased market code. A type hit wins over a
 *          market hit; an unmatched security uses the caller's fallback.
 * @ingroup TradeManage
 */
class HKU_API RateTable {
public:
    /**
     * Rebuild the table; a no-op when the expression is unchanged
     * @details Items are staged in locals and committed only if the whole expression is valid, so a
     *          rejected expression leaves the table unchanged.
     * @param src the "key:ratio" list, an empty list giving an empty table
     * @param tag the name of the option being parsed, prefixing every message
     * @param allow_zero whether a ratio of 0 is valid
     * @param allow_one whether a ratio of 1 is valid
     * @param allow_above_one whether a ratio above 1 is valid, for rates not a fraction of the
     *                        security amount (e.g. an exchange rate)
     * @throw when an item is malformed or a ratio falls outside the allowed range
     */
    void parse(const string& src, const string& tag, bool allow_zero, bool allow_one,
               bool allow_above_one = false);

    /** The ratio of the security, fallback when no key matches */
    double lookup(const Stock& stock, double fallback) const;

    /** Whether a type key or a market key of the table matches the security */
    bool has(const Stock& stock) const;

    /** The smallest ratio of the table, fallback when the table is empty */
    double minRate(double fallback) const;

    bool empty() const {
        return m_type_rates.empty() && m_market_rates.empty();
    }

private:
    string m_src;  // the expression the maps were built from
    std::unordered_map<uint32_t, double> m_type_rates;
    std::unordered_map<string, double> m_market_rates;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_RATE_TABLE_H_ */
