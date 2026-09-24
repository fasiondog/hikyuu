/*
 * MarketInfo.h
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#pragma once
#ifndef MARKETINFO_H_
#define MARKETINFO_H_

#include "DataType.h"

namespace hku {

/**
 * Market information record
 * @ingroup StockManage
 */
class HKU_API MarketInfo {
public:
    /** Default constructor, returns Null<MarketInfo>() */
    MarketInfo();

    /**
     * @param market market abbreviation
     * @param name market name
     * @param description market description
     * @param code base index: used to read the trading calendar of the market
     * @param lastDate current last date of the market
     * @param openTime1 start time of trading session 1
     * @param closeTime1 end time of trading session 1
     * @param openTime2 start time of trading session 2
     * @param closeTime2 end time of trading session 2
     */
    MarketInfo(const string& market, const string& name, const string& description,
               const string& code, const Datetime& lastDate, TimeDelta openTime1,
               TimeDelta closeTime1, TimeDelta openTime2, TimeDelta closeTime2);

    MarketInfo(const MarketInfo&) = default;
    MarketInfo& operator=(const MarketInfo&) = default;

    MarketInfo(MarketInfo&&) noexcept;
    MarketInfo& operator=(MarketInfo&&) noexcept;

    /** Get the market abbreviation */
    const string& market() const noexcept {
        return m_market;
    }

    /** Get the market name */
    const string& name() const noexcept {
        return m_name;
    }

    /** Get the market description */
    const string& description() const noexcept {
        return m_description;
    }

    /** Get the index code corresponding to the market */
    const string& code() const noexcept {
        return m_code;
    }

    /** Get the last update date of the market data */
    Datetime lastDate() const noexcept {
        return m_lastDate;
    }

    /** Opening time of session 1 */
    TimeDelta openTime1() const noexcept {
        return m_openTime1;
    }

    /** Closing time of session 1 */
    TimeDelta closeTime1() const noexcept {
        return m_closeTime1;
    }

    /** Opening time of session 2 */
    TimeDelta openTime2() const noexcept {
        return m_openTime2;
    }

    /** Closing time of session 2 */
    TimeDelta closeTime2() const noexcept {
        return m_closeTime2;
    }

    /** Used by __str__ of python only */
    string toString() const;

private:
    string m_market;         // Market identifier
    string m_name;           // Market name
    string m_description;    // Description
    string m_code;           // Index code of the market, used to get the trading calendar
    Datetime m_lastDate;     // Current last date of the market
    TimeDelta m_openTime1;   // Morning opening time
    TimeDelta m_closeTime1;  // Morning closing time
    TimeDelta m_openTime2;   // Afternoon opening time
    TimeDelta m_closeTime2;  // Afternoon closing time
};

/**
 * Output the market information, e.g.:
 * MarketInfo(SH, Shanghai Stock Exchange, Shanghai market, 000001, 2011-Dec-06 00:00:00)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const MarketInfo&);

///////////////////////////////////////////////////////////////////////////////
//
// Relational comparison functions
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const MarketInfo&, const MarketInfo&);
bool operator!=(const MarketInfo&, const MarketInfo&);

/** Equal comparison */
inline bool operator==(const MarketInfo& m1, const MarketInfo& m2) {
    return m1.market() == m2.market();
}

/** Unequal comparison */
inline bool operator!=(const MarketInfo& m1, const MarketInfo& m2) {
    return m1.market() != m2.market();
}

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MarketInfo> : ostream_formatter {};
#endif

#endif /* MARKETINFO_H_ */
