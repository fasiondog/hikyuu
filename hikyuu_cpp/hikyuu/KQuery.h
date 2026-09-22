/*
 * KQuery.h
 *
 *  Created on: 2009-11-23
 *      Author: fasiondog
 */

#pragma once
#ifndef KQUERY_H_
#define KQUERY_H_

#include "KRecord.h"

namespace hku {

/**
 * Query condition for K-line (candlestick) data by index
 * @ingroup StockManage
 */
class HKU_API KQuery {
public:
    /// Query mode: by index or by date
    enum QueryType : uint8_t {
        INDEX = 0,  ///< Query by index
        DATE = 1,   ///< Query by date
        INVALID = 2
    };

    typedef string KType;

    // Basic K-line types
    static const string MIN;
    static const string MIN5;
    static const string MIN15;
    static const string MIN30;
    static const string MIN60;
    static const string HOUR2;
    static const string DAY;
    static const string WEEK;
    static const string MONTH;
    static const string QUARTER;
    static const string HALFYEAR;
    static const string YEAR;

    // Extended K-line types
    static const string DAY3;
    static const string DAY5;
    static const string DAY7;
    static const string MIN3;
    static const string HOUR4;   // Not supported by default
    static const string HOUR6;   // Not supported by default
    static const string HOUR12;  // Not supported by default

    static const string TIMELINE;  // Intraday time-line
    static const string TRANS;     // Tick

    /** Whether the given K-line type is valid */
    static bool isValidKType(const string& ktype);

    /** Whether it is a base ktype */
    static bool isBaseKType(const string& ktype) noexcept;

    /** Whether it is an extended ktype */
    static bool isExtraKType(const string& ktype);

    /** Get all base KTypes */
    static vector<KType> getBaseKTypeList() noexcept;

    /** Get all extended KTypes */
    static vector<KType> getExtraKTypeList();

    static int32_t getKTypeInMin(const KType& ktype);

    static int32_t getBaseKTypeInMin(const KType& ktype) noexcept;

    static int64_t getKTypeInSeconds(const KType& ktype);

    /**
     * Price adjustment type
     * @note Periods above daily, e.g. weekly / monthly, do not support price adjustment
     */
    enum RecoverType : uint8_t {
        NO_RECOVER = 0,      ///< No adjustment
        FORWARD = 1,         ///< Forward adjustment
        BACKWARD = 2,        ///< Backward adjustment
        EQUAL_FORWARD = 3,   ///< Equal-ratio forward adjustment
        EQUAL_BACKWARD = 4,  ///< Equal-ratio backward adjustment
        INVALID_RECOVER_TYPE = 5
    };

    /** Default constructor: query all daily data by index, without price adjustment */
    KQuery()
    : m_start(0),
      m_end(Null<int64_t>()),
      m_queryType(INDEX),
      m_dataType(DAY),
      m_recoverType(NO_RECOVER) {};

    /**
     * Query K-line data over the range [start, end)
     * @param start start index, negative values are supported
     * @param end  end index (exclusive), negative values are supported
     * @param dataType K-line type
     * @param recoverType price adjustment type
     * @param queryType query by index by default
     */
    KQuery(int64_t start,  // cppcheck-suppress [noExplicitConstructor]
           int64_t end = Null<int64_t>(), const KType& dataType = DAY,
           RecoverType recoverType = NO_RECOVER, QueryType queryType = INDEX)
    : m_start(start),
      m_end(end),
      m_queryType(queryType),
      m_dataType(dataType),
      m_recoverType(recoverType) {
        to_upper(m_dataType);
    }

    /**
     * Query K-line data by date, over the range [start, end)
     * @param start start date
     * @param end  end date
     * @param ktype K-line type
     * @param recoverType price adjustment type
     */
    KQuery(Datetime start,  // cppcheck-suppress [noExplicitConstructor]
           Datetime end = Null<Datetime>(), const KType& ktype = DAY,
           RecoverType recoverType = NO_RECOVER);

    /**
     * Return the specified start index when querying by index, otherwise Null<int64_t>()
     */
    int64_t start() const noexcept {
        return m_queryType != INDEX ? Null<int64_t>() : m_start;
    }

    /**
     * Return the specified end index when querying by index, otherwise Null<int64_t>()
     */
    int64_t end() const noexcept {
        return m_queryType != INDEX ? Null<int64_t>() : m_end;
    }

    /**
     * Return the specified start date when querying by date, otherwise Null<Datetime>()
     */
    Datetime startDatetime() const;

    /**
     * Return the specified end date when querying by date, otherwise Null<Datetime>()
     */
    Datetime endDatetime() const;

    /** Get the query condition type */
    QueryType queryType() const noexcept {
        return m_queryType;
    }

    /** Get the K-line data type */
    // KType kType() const { return m_dataType; }
    const string& kType() const noexcept {
        return m_dataType;
    }

    /** Get the number of seconds corresponding to the K-line data type */
    TimeDelta kTypeInSeconds() const {
        return Seconds(getKTypeInSeconds(m_dataType));
    }

    /** Get the price adjustment type */
    RecoverType recoverType() const noexcept {
        return m_recoverType;
    }

    /** Set the price adjustment type */
    void recoverType(RecoverType recoverType) noexcept {
        m_recoverType = recoverType;
    }

    /**
     * @brief Hash value (use with care)
     * @note When end is null, the behavior of fetching K-line data is inconsistent, so do not use
     *       this method unless you know your use case
     * @return size_t
     */
    uint64_t hash() const;

    /** Whether it is a right-open interval, i.e. no end time was specified */
    bool isRightOpening() const {
        if (m_queryType == DATE) {
            return endDatetime().isNull();
        }
        return m_end == Null<int64_t>();
    }

    /** Get the name of the queryType, used for display output */
    static string getQueryTypeName(QueryType);

    /** Get the name of the KType, used for display output */
    static string getKTypeName(const KType&);

    /** Get the name of the recoverType, used for display output */
    static string getRecoverTypeName(RecoverType);

    /** Get the queryType enum value matching the given string name */
    static QueryType getQueryTypeEnum(const string&);

    /** Get the KType enum value matching the given string name */
    static KType getKTypeEnum(const string&);

    /** Get the recoverType enum value matching the given string name */
    static RecoverType getRecoverTypeEnum(const string&);

private:
    int64_t m_start;
    int64_t m_end;
    QueryType m_queryType;
    KType m_dataType;
    RecoverType m_recoverType;
};

/**
 * Create a K-line query by index, over the range [start, end)
 * @param start start index, negative values are supported
 * @param end  end index (exclusive), negative values are supported
 * @param dataType K-line type
 * @param recoverType price adjustment type
 * @see KQuery
 * @ingroup StockManage*
 */
KQuery HKU_API KQueryByIndex(int64_t start = 0, int64_t end = Null<int64_t>(),
                             const KQuery::KType& dataType = KQuery::DAY,
                             KQuery::RecoverType recoverType = KQuery::NO_RECOVER);

inline KQuery KQueryByIndex(int64_t start, int64_t end, const KQuery::KType& dataType,
                            KQuery::RecoverType recoverType) {
    return KQuery(start, end, dataType, recoverType, KQuery::INDEX);
}

/**
 * Create a K-line query by date, over the range [startDatetime, endDatetime)
 * @param start start date
 * @param end  end date (exclusive)
 * @param dataType K-line type
 * @param recoverType price adjustment type
 * @see KQuery
 * @ingroup StockManage
 */
KQuery HKU_API KQueryByDate(const Datetime& start = Datetime::min(),
                            const Datetime& end = Null<Datetime>(),
                            const KQuery::KType& dataType = KQuery::DAY,
                            KQuery::RecoverType recoverType = KQuery::NO_RECOVER);

inline KQuery KQueryByDate(const Datetime& start, const Datetime& end,
                           const KQuery::KType& dataType, KQuery::RecoverType recoverType) {
    return KQuery(start, end, dataType, recoverType);
}

/**
 * Print the KQuery information, e.g. KQuery(start, end, queryType, kType, recoverType)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream& os, const KQuery& query);

///////////////////////////////////////////////////////////////////////////////
//
// Relational comparison functions. They are not defined inside the class so that
// Null<>() == d is supported, i.e. Null can be placed on the left side
//
///////////////////////////////////////////////////////////////////////////////
bool HKU_API operator==(const KQuery&, const KQuery&) noexcept;
bool HKU_API operator!=(const KQuery&, const KQuery&) noexcept;

/**
 * Provide the Null value of KQuery
 * @ingroup StockManage
 */
template <>
class Null<KQuery> {
public:
    Null() {}
    operator KQuery() {
        return KQuery(Null<int64_t>(), Null<int64_t>(),
                      "",  // KQuery::INVALID_KTYPE,
                      KQuery::INVALID_RECOVER_TYPE, KQuery::INVALID);
    }
};

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::KQuery> : ostream_formatter {};
#endif

#endif /* KQUERY_H_ */
