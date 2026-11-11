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
 * A "key:ratio" lookup table, shared by the margin models and the cost models
 * @details The expression is a list of "key:ratio" items separated by ",", ";" or blanks, e.g.
 *          "12:0.10, SHF:0.12". An all-digit key is a security type, any other key is a market
 *          code (compared upper-cased). A type hit wins over a market hit, and an unmatched
 *          security falls back to the rate the caller supplies.
 * @ingroup TradeManage
 */
class HKU_API RateTable {
public:
    /**
     * Rebuild the table, doing nothing when the expression is the one already parsed
     * @details The items are parsed into locals and committed only when the whole expression is
     *          valid, so a rejected string never leaves a partially filled table behind.
     * @param src the "key:ratio" list, an empty list giving an empty table
     * @param tag the name of the option being parsed, prefixing every message
     * @param allow_zero whether a ratio of 0 is valid
     * @param allow_one whether a ratio of 1 is valid
     * @param allow_above_one whether a ratio above 1 is valid, for a rate that is not a fraction
     *                        of the security amount (e.g. an exchange rate)
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
