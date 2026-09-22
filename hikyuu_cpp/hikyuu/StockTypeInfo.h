/*
 * StockTypeInfo.h
 *
 *  Created on: 2011-12-12
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCKTYPEINFO_H_
#define STOCKTYPEINFO_H_

#include "DataType.h"

namespace hku {

#define STOCKTYPE_BLOCK 0    /// Block (sector)
#define STOCKTYPE_A 1        /// A-share
#define STOCKTYPE_INDEX 2    /// Index
#define STOCKTYPE_B 3        /// B-share
#define STOCKTYPE_FUND 4     /// Fund
#define STOCKTYPE_ETF 5      /// ETF
#define STOCKTYPE_ND 6       /// Treasury bond
#define STOCKTYPE_BOND 7     /// Bond
#define STOCKTYPE_GEM 8      /// ChiNext (Growth Enterprise Market)
#define STOCKTYPE_START 9    /// STAR Market
#define STOCKTYPE_CRYPTO 10  /// Cryptocurrency
#define STOCKTYPE_A_BJ 11    /// Beijing Stock Exchange (its minimum trading unit is not 100 shares)

#define STOCKTYPE_TMP 999  /// Used for a temporary Stock

/**
 * Security type information
 * @ingroup StockManage
 */
class HKU_API StockTypeInfo {
public:
    /** Default constructor, returns Null<StockTypeInfo>() */
    StockTypeInfo();
    StockTypeInfo(uint32_t, const string&, price_t, price_t, int, double, double);

    StockTypeInfo(const StockTypeInfo&) = default;
    StockTypeInfo& operator=(const StockTypeInfo&) = default;

    StockTypeInfo(StockTypeInfo&&) noexcept;
    StockTypeInfo& operator=(StockTypeInfo&&) noexcept;

    /** Get the security type */
    uint32_t type() const noexcept {
        return m_type;
    }

    /** Get the description of the security type */
    const string& description() const noexcept {
        return m_description;
    }

    /** Get the minimum tick size */
    price_t tick() const noexcept {
        return m_tick;
    }

    /** Price per tick */
    price_t tickValue() const noexcept {
        return m_tickValue;
    }

    /** Price per unit = tickValue / tick */
    price_t unit() const noexcept {
        return m_unit;
    }

    /** Get the price precision */
    int precision() const noexcept {
        return m_precision;
    }

    /** Get the minimum trade quantity per order */
    double minTradeNumber() const noexcept {
        return m_minTradeNumber;
    }

    /** Get the maximum trade quantity per order */
    double maxTradeNumber() const noexcept {
        return m_maxTradeNumber;
    }

    /** Used by __str__ of python only */
    string toString() const;

private:
    uint32_t m_type;          // Security type
    string m_description;     // Description
    price_t m_tick;           // Minimum tick size
    price_t m_tickValue;      // Price of every tick
    price_t m_unit;           // Price per minimum change, i.e. unit price = tickValue / tick
    int m_precision;          // Price precision
    double m_minTradeNumber;  // Minimum trade quantity per order
    double m_maxTradeNumber;  // Maximum trade quantity per order
};

/**
 * Output the security type information, e.g. StockTypeInfo(type, description, tick, precision,
 * minTradeNumber, maxTradeNumber)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const StockTypeInfo&);

///////////////////////////////////////////////////////////////////////////////
//
// Relational comparison functions
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const StockTypeInfo&, const StockTypeInfo&);
bool operator!=(const StockTypeInfo&, const StockTypeInfo&);

/** Equal comparison */
inline bool operator==(const StockTypeInfo& m1, const StockTypeInfo& m2) {
    return m1.type() == m2.type();
}

/** Unequal comparison */
inline bool operator!=(const StockTypeInfo& m1, const StockTypeInfo& m2) {
    return m1.type() != m2.type();
}

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::StockTypeInfo> : ostream_formatter {};
#endif

#endif /* STOCKTYPEINFO_H_ */
