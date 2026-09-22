/*
 * Stock.h
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCK_H_
#define STOCK_H_

#include <shared_mutex>
#include "StockWeight.h"
#include "KQuery.h"
#include "TimeLineRecord.h"
#include "TransRecord.h"
#include "HistoryFinanceInfo.h"

namespace hku {

class HKU_API StockManager;
class KDataDriverConnect;

template <class DriverConnectT>
class DriverConnectPool;

typedef DriverConnectPool<KDataDriverConnect> KDataDriverConnectPool;
typedef shared_ptr<KDataDriverConnectPool> KDataDriverConnectPoolPtr;
class HKU_API KData;
class HKU_API Parameter;
class HKU_API Block;
class HKU_API KDataSharedBufferImp;

/**
 * Base class of a security (Stock). Applications usually operate on it through StockPtr.
 * @ingroup StockManage
 */
class HKU_API Stock {
    friend class StockManager;
    friend class KDataSharedBufferImp;

private:
    static const string default_market;
    static const string default_code;
    static const string default_market_code;
    static const string default_name;
    static const uint32_t default_type;
    static const bool default_valid;
    static const Datetime default_startDate;
    static const Datetime default_lastDate;
    static const price_t default_tick;
    static const price_t default_tickValue;
    static const price_t default_unit;
    static const int default_precision;
    static const size_t default_minTradeNumber;
    static const size_t default_maxTradeNumber;

public:
    Stock();

    Stock(const Stock&) noexcept;
    Stock(Stock&&) noexcept;
    Stock(const string& market, const string& code, const string& name);

    Stock(const string& market, const string& code, const string& name, uint32_t type, bool valid,
          const Datetime& startDate, const Datetime& lastDate);
    Stock(const string& market, const string& code, const string& name, uint32_t type, bool valid,
          const Datetime& startDate, const Datetime& lastDate, price_t tick, price_t tickValue,
          int precision, size_t minTradeNumber, size_t maxTradeNumber);
    virtual ~Stock();
    Stock& operator=(const Stock&);
    Stock& operator=(Stock&&) noexcept;
    bool operator==(const Stock&) const;
    bool operator!=(const Stock&) const;

    /**
     * Get the internal id, usually used as a map key. The id is actually the memory address of
     * m_data.
     * @note It is not the stockid stored in the database.
     */
    uint64_t id() const noexcept;

    /** Get the market abbreviation, which uniquely identifies the market */
    const string& market() const noexcept;

    /** Get the security code */
    const string& code() const noexcept;

    /** Market abbreviation + security code, e.g. sh000001 */
    const string& market_code() const noexcept;

    /** Get the security name */
    const string& name() const noexcept;

    /** Get the security type */
    uint32_t type() const noexcept;

    /** Whether the security is currently valid */
    bool valid() const noexcept;

    /** Get the start date of the security */
    const Datetime& startDatetime() const noexcept;

    /** Get the last date of the security */
    const Datetime& lastDatetime() const noexcept;

    /** Get the minimum tick size */
    price_t tick() const noexcept;

    /** Value of the minimum tick */
    price_t tickValue() const noexcept;

    /** Value per unit = tickValue / tick */
    price_t unit() const noexcept;

    /** Get the price precision */
    int precision() const noexcept;

    /** Get the minimum trade quantity, same as minTradeNumber */
    double atom() const noexcept;

    /** Get the minimum trade quantity */
    double minTradeNumber() const noexcept;

    /** Get the maximum trade quantity */
    double maxTradeNumber() const noexcept;

    void market(const string& market_);
    void code(const string& code_);
    void name(const string& name_);
    void type(uint32_t type_);
    void valid(bool valid_);
    void precision(int precision_);
    void startDatetime(const Datetime&);
    void lastDatetime(const Datetime&);
    void tick(price_t tick_);
    void tickValue(price_t val);
    void minTradeNumber(double num);
    void maxTradeNumber(double num);
    void atom(double num) {
        return minTradeNumber(num);
    }

    /**
     * Get the equity/dividend adjustment (weight) information within [start, end)
     * @param start start date
     * @param end end date
     * @return the list of weight records that satisfy the condition
     * @note The weight cache strategy depends on the process role:
     *       - Client mode (IPC): at startup, the full weight set is materialized according to the
     *         load_stock_weight config (the source is the shared-memory snapshot published by the
     *         main process: IpcBaseInfoDriver shm first, falling back to IPC/local when not
     *         covered), so getWeight hits the local cache directly. Only when the config is
     *         disabled (load_stock_weight=false), or for securities that were not materialized
     *         (added by addStock, newly constructed, ...), the whole weight list of the security is
     *         read through the driver on first access and then cached (an empty result is cached as
     *         well, to avoid repeated empty queries).
     *       - shm server role: after the base info snapshot containing finance data is published,
     *         StockManager::releaseShmServerBaseInfoCache() releases the local weight cache of each
     *         security to reclaim memory (clients read through shared memory, so the server does not
     *         need to keep a copy). Later access to a released security (IPC fallback reply, or the
     *         in-process API) lazily reloads and caches it again, so the result stays correct.
     *       - Plain standalone main process: only the local cache materialized at startup is read,
     *         with no lazy-loading fallback.
     */
    StockWeightList getWeight(const Datetime& start = Datetime::min(),
                              const Datetime& end = Null<Datetime>()) const;

    /** Get the number of K-line (candlestick) bars of the given type */
    size_t getCount(KQuery::KType dataType = KQuery::DAY) const;

    /** Get the market value at the given datetime, i.e. the close price of the last record whose
     *  date is not later than the given datetime; 0 if the security is no longer valid */
    price_t getMarketValue(const Datetime&, KQuery::KType) const;

    /**
     * Get the K-line index range that matches the condition specified by the query
     * @param query [in] the query condition
     * @param out_start [out] the start index of the range
     * @param out_end [out] the end index of the range, exclusive
     * @return true on success | false on failure
     */
    bool getIndexRange(const KQuery& query, size_t& out_start, size_t& out_end) const;

    /** Get the K-line record at the given index; returns KRecord::NullRecord if pos is invalid */
    KRecord getKRecord(size_t pos, const KQuery::KType& dataType = KQuery::DAY) const;

    /** Get the KRecord of the given date according to the data type (daily / weekly / ...) */
    KRecord getKRecord(const Datetime&, const KQuery::KType& ktype = KQuery::DAY) const;

    /** Get the K-line data */
    KData getKData(const KQuery&) const;

    /**
     * Get the KRecordList that matches the query condition; not recommended for direct use in a
     * client
     * @note This method does not support price adjustment
     * @param query the query condition
     */
    KRecordList getKRecordList(const KQuery& query) const;

    /** Get the date list */
    DatetimeList getDatetimeList(const KQuery& query) const;

    /** Get the intraday time-line data */
    TimeLineList getTimeLineList(const KQuery& query) const;

    /** Get the historical tick (transaction) data */
    TransList getTransList(const KQuery& query) const;

    /**
     * Get the current financial information
     */
    Parameter getFinanceInfo() const;

    /**
     * Get the sectors that this security belongs to
     * @param category the sector category; if empty, the sectors of all categories are returned
     * @return BlockList
     */
    vector<Block> getBelongToBlockList(const string& category) const;

    /**
     * Get the historical financial information
     * @note Returns a copy of the historical financial records. The main process (not in client
     *       mode) returns a copy of its materialized internal cache; after that cache is released by
     *       StockManager::releaseShmServerBaseInfoCache() (invoked by the shm server role once the
     *       snapshot containing finance data has been published), it is lazily reloaded on demand.
     *       In client mode (IPC) it is not materialized locally; instead it is read on demand
     *       through the base info driver from the shared-memory snapshot published by the main
     *       process (IpcBaseInfoDriver shm first, falling back to IPC/local when not covered), so
     *       that it does not duplicate the snapshot in the client's memory.
     */
    vector<HistoryFinanceInfo> getHistoryFinance() const;

    /**
     * Get the trading calendar of its own market (not the trading dates of the security itself)
     * @param query
     * @return DatetimeList
     */
    DatetimeList getTradingCalendar(const KQuery& query) const;

    /**
     * Whether the given time falls inside the trading session; the date part is ignored and only
     * the hour/minute/second is checked
     * @param time the time to check
     */
    bool isTransactionTime(Datetime time);

    /** Set the K-line data driver */
    void setKDataDriver(const KDataDriverConnectPoolPtr& kdataDriver);

    /** Get the K-line driver */
    KDataDriverConnectPoolPtr getKDataDirver() const;

    /**
     * Cache the K-line data inside the security itself
     * @note Generally not called directly; use with care
     */
    void loadKDataToBuffer(KQuery::KType) const;

    /** Get a full copy of the cached K-line data of the given type; empty if not cached */
    KRecordList getKRecordListFromBuffer(KQuery::KType) const;

    /** Release the K-line cache of the given type */
    void releaseKDataBuffer(KQuery::KType) const;

    /** Whether the K-line data of the given type is cached */
    bool isBuffer(KQuery::KType) const noexcept;

    bool isPreload(KQuery::KType ktype) const noexcept;

    /** Whether it is Null */
    bool isNull() const noexcept;

    /** (Temporary function) only used to update the K-line data in the cache */
    void realtimeUpdate(KRecord, const KQuery::KType& ktype = KQuery::DAY);

    /** Get the last update time of the given K-line data type */
    Datetime getLastUpdateTime(const KQuery::KType& ktype) const;

    /**
     * Set the KRecordList directly, for some temporarily created Stock instances
     * @note Call with care; usually used by an external data source to set the data directly
     */
    void setKRecordList(const KRecordList& ks, const KQuery::KType& ktype = KQuery::DAY);
    void setKRecordList(KRecordList&& ks, const KQuery::KType& ktype = KQuery::DAY);

    /** Only used by __str__ in python */
    string toString() const;

private:
    bool _getIndexRangeByIndex(const KQuery&, size_t& out_start, size_t& out_end) const;

    // The following functions are basic operations guarded by a read lock
    const KRecord& _getKRecordFromBuffer(size_t pos, const KQuery::KType& ktype) const;
    KRecordList _getKRecordListFromBuffer(size_t start_ix, size_t end_ix,
                                          KQuery::KType ktype) const;

    bool _getIndexRangeFromBuffer(const KQuery& query, size_t& out_start, size_t& out_end) const;
    bool _getIndexRangeByIndexFromBuffer(const KQuery& query, size_t& out_start,
                                         size_t& out_end) const;
    bool _getIndexRangeByDateFromBuffer(const KQuery&, size_t&, size_t&) const;

    KRecordList _getKRecordList(const KQuery& query) const;

    // Only called by StockManager during initialization
    void setPreload(const vector<KQuery::KType>& preload_ktypes);

    void loadKDataToBufferFromKRecordList(const KQuery::KType& ktype, KRecordList&& ks) const;
    void setHistoryFinance(vector<HistoryFinanceInfo>&&);

private:
    struct HKU_API Data;
    shared_ptr<Data> m_data;
    KDataDriverConnectPoolPtr m_kdataDriver;
};

struct HKU_API Stock::Data {
    string m_market;       // The market abbreviation it belongs to
    string m_code;         // Security code
    string m_market_code;  // Market abbreviation + security code
    string m_name;         // Security name
    uint32_t m_type;       // Security type
    bool m_valid;          // Whether the security is currently valid
    Datetime m_startDate;  // Start date of the security
    Datetime m_lastDate;   // Last date of the security

    StockWeightList m_weightList;  // Equity/dividend adjustment (weight) record list
    std::shared_mutex m_weight_mutex;
    // Whether the weight data has been initialized (materialized at startup or lazily loaded as a
    // fallback; it may be empty). When it is set:
    // - Client mode: set when materialized at startup while load_stock_weight is enabled; set on the
    //   first lazy load for securities that were not materialized, e.g. added by addStock or newly
    //   constructed, when the config is disabled;
    // - shm server role: set to false by releaseShmServerBaseInfoCache() after the snapshot
    //   containing finance data is published (the cache has been released and the memory returned),
    //   then set again after the next access lazily reloads it through the driver;
    // An empty result sets it as well, so securities without weight data do not hit the driver again
    // on every query.
    mutable std::atomic_bool m_weight_ready{false};

    mutable vector<HistoryFinanceInfo>
      m_history_finance;  // Historical financial info [report date, field 1, field 2, ...]
    // Whether the historical finance data has been initialized (set when the main process preloads it
    // at startup; set to false after it is released by releaseShmServerBaseInfoCache(), and set again
    // after the next access lazily reloads it; in client mode it is never materialized locally, so it
    // stays false)
    mutable std::atomic_bool m_history_finance_ready{false};
    mutable std::shared_mutex m_history_finance_mutex;

    price_t m_tick;
    price_t m_tickValue;
    price_t m_unit;
    int m_precision;
    double m_minTradeNumber;
    double m_maxTradeNumber;

    std::unordered_set<string> m_ktype_preload;  // Records whether the K-line data of this security
                                                 // needs to be preloaded
    unordered_map<string, KRecordList*> pKData;
    unordered_map<string, std::shared_mutex*> pMutex;
    unordered_map<string, Datetime> m_lastUpdate;  // Last update time of each K-line data type

    Data();
    Data(const string& market, const string& code, const string& name, uint32_t type, bool valid,
         const Datetime& startDate, const Datetime& lastDate, price_t tick, price_t tickValue,
         int precision, double minTradeNumber, double maxTradeNumber);
    string marketCode() const;
    virtual ~Data();
};

/**
 * Print the Stock information, e.g. Stock(market, code, name, type, valid, startDatetime,
 * lastDatetime)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream& os, const Stock& stock);

/** @ingroup StockManage */
typedef vector<Stock> StockList;

/**
 * Get a Stock instance; it wraps StockManager so that a client does not use the StockManager object
 * directly
 * @param querystr in the form of "market abbreviation + security code", e.g. "sh000001"
 * @return the corresponding security instance; Null<Stock>() if it does not exist, no exception is
 *         thrown
 * @ingroup StockManage
 */
Stock HKU_API getStock(const string& querystr);

/* Used to take a Stock instance as a map key; it is generally recommended to use stock.id as the key,
 * otherwise the map has to build a new object through the copy constructor, which is inefficient */
bool operator<(const Stock& s1, const Stock& s2);
inline bool operator<(const Stock& s1, const Stock& s2) {
    return s1.id() < s2.id();
}

inline uint64_t Stock::id() const noexcept {
    return isNull() ? 0 : (int64_t)m_data.get();
}

inline bool Stock::operator!=(const Stock& stock) const {
    return !(*this == stock);
}

inline bool Stock::isNull() const noexcept {
    return !m_data || !m_kdataDriver;
}

}  // namespace hku

namespace std {
template <>
class hash<hku::Stock> {
public:
    size_t operator()(hku::Stock const& stk) const noexcept {
        return stk.id();
    }
};
}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::Stock> : ostream_formatter {};
#endif

#endif /* STOCK_H_ */
