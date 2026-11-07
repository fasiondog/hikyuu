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
    /** The offset in minutes of the unified expression (Beijing wall clock, UTC+8) */
    static constexpr int UNIFIED_TZ_OFFSET_MIN = 480;

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
     * @param tzOffsetMin standard UTC offset in minutes (e.g. 480 for UTC+8)
     * @param dstRule daylight saving rule: 0=none, 1=US, 2=EU
     * @param dataLag visibility delay in trading days relative to the base market
     */
    MarketInfo(const string& market, const string& name, const string& description,
               const string& code, const Datetime& lastDate, TimeDelta openTime1,
               TimeDelta closeTime1, TimeDelta openTime2, TimeDelta closeTime2,
               int tzOffsetMin = 480, int dstRule = 0, int dataLag = 0);

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

    /** Standard UTC offset in minutes (e.g. 480 for UTC+8) */
    int tzOffsetMin() const noexcept {
        return m_tzOffsetMin;
    }

    /** Daylight saving rule: 0=none, 1=US, 2=EU */
    int dstRule() const noexcept {
        return m_dstRule;
    }

    /** Visibility delay in trading days relative to the base market */
    int dataLag() const noexcept {
        return m_dataLag;
    }

    /** Whether all the fields are equal (operator== only compares the market) */
    bool equals(const MarketInfo& rhs) const;

    /**
     * Convert a market-local wall-clock datetime into the unified expression (Beijing)
     * @details CN / HK (offset 480, no daylight) returns the input unchanged; null in, null out
     */
    Datetime localToUnified(const Datetime&) const;

    /**
     * Convert a unified expression (Beijing) datetime back to the market-local wall clock
     * @details Exact inverse of localToUnified; the repeated local hour of a daylight transition
     * resolves to the standard side (which is why the constructor rejects such sessions)
     */
    Datetime unifiedToLocal(const Datetime&) const;

    /** Whether the market trades in the ambiguous local hour [01:00,02:00) of its daylight rule */
    bool hasAmbiguousSession() const;

    /** Whether the local wall clock already is the unified expression (CN / HK identity) */
    bool isUnifiedClock() const noexcept {
        return m_tzOffsetMin == UNIFIED_TZ_OFFSET_MIN && m_dstRule == 0;
    }

    /** Used by __str__ of python only */
    string toString() const;

private:
    /** Whether the given UTC instant falls inside the daylight period of the market's rule */
    bool _isDstUTC(const Datetime& utc) const;

    string m_market;         // Market identifier
    string m_name;           // Market name
    string m_description;    // Description
    string m_code;           // Index code of the market, used to get the trading calendar
    Datetime m_lastDate;     // Current last date of the market
    TimeDelta m_openTime1;   // Morning opening time
    TimeDelta m_closeTime1;  // Morning closing time
    TimeDelta m_openTime2;   // Afternoon opening time
    TimeDelta m_closeTime2;  // Afternoon closing time
    int m_tzOffsetMin{UNIFIED_TZ_OFFSET_MIN};  // Standard UTC offset in minutes
    int m_dstRule{0};                          // Daylight saving rule: 0=none, 1=US, 2=EU
    int m_dataLag{0};  // Visibility delay in trading days relative to the base market
};

/**
 * Output the market information, e.g.:
 * MarketInfo(SH, Shanghai Stock Exchange, Shanghai market, 000001, 2011-Dec-06 00:00:00, 9:30,
 * 11:30, 13:0, 15:0, 480, 0, 0)
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
