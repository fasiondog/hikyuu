/*
 * BaseInfoDriver.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#pragma once
#ifndef BASEINFODRIVER_H_
#define BASEINFODRIVER_H_

#include <unordered_set>
#include "../utilities/Parameter.h"
#include "../MarketInfo.h"
#include "../StockTypeInfo.h"
#include "../Stock.h"
#include "../ZhBond10.h"
#include "../HistoryFinanceInfo.h"
#include "../utilities/db_connect/SQLStatementBase.h"

namespace hku {

struct StockInfo {
    StockInfo()
    : type(Null<uint32_t>()),
      valid(0),
      startDate(0),
      endDate(0),
      precision(1),
      tick(0.0),
      tickValue(0.0),
      minTradeNumber(0.0),
      maxTradeNumber(0.0) {}

    StockInfo(const StockInfo&) = default;
    StockInfo& operator=(const StockInfo&) = default;

    StockInfo(StockInfo&& rhs)
    : market(std::move(rhs.market)),
      code(std::move(rhs.code)),
      name(std::move(rhs.name)),
      type(rhs.type),
      valid(rhs.valid),
      startDate(rhs.startDate),
      endDate(rhs.endDate),
      precision(rhs.precision),
      tick(rhs.tick),
      tickValue(rhs.tickValue),
      minTradeNumber(rhs.minTradeNumber),
      maxTradeNumber(rhs.maxTradeNumber) {}

    StockInfo& operator=(StockInfo&& rhs) {
        if (this != &rhs) {
            market = std::move(rhs.market);
            code = std::move(rhs.code);
            name = std::move(rhs.name);
            type = rhs.type;
            valid = rhs.valid;
            startDate = rhs.startDate;
            endDate = rhs.endDate;
            precision = rhs.precision;
            tick = rhs.tick;
            tickValue = rhs.tickValue;
            minTradeNumber = rhs.minTradeNumber;
            maxTradeNumber = rhs.maxTradeNumber;
        }
        return *this;
    }

    static const char* getSelectSQL() {
        return "select c.market, a.code, a.name, a.type, a.valid, a.startDate, a.endDate, b.tick, "
               "b.tickValue, b.precision, b.minTradeNumber, b.maxTradeNumber from stock a, "
               "stocktypeinfo b, market c where a.type = b.id and a.marketid = c.marketid";
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, market, code, name, type, valid, startDate, endDate, tick, tickValue,
                      precision, minTradeNumber, maxTradeNumber);
    }

    string market;
    string code;
    string name;
    uint32_t type;
    uint32_t valid;
    uint64_t startDate;
    uint64_t endDate;
    uint32_t precision;
    double tick;
    double tickValue;
    double minTradeNumber;
    double maxTradeNumber;
};

/**
 * Base class of the basic information data driver
 * @ingroup DataDriver
 */
class HKU_API BaseInfoDriver {
    PARAMETER_SUPPORT

public:
    typedef unordered_map<string, MarketInfo> MarketInfoMap;
    typedef unordered_map<uint32_t, StockTypeInfo> StockTypeInfoMap;

    /**
     * Constructor
     * @param name driver name
     */
    BaseInfoDriver(const string& name);
    virtual ~BaseInfoDriver() {}

    /** Get the driver name */
    const string& name() const;

    /**
     * Driver initialization
     * @param params
     * @return
     */
    bool init(const Parameter& params);

    /**
     * Driver initialization; when it is implemented concretely, attention should be paid to closing
     * the related resources opened before.
     * @return
     */
    virtual bool _init() = 0;

    /**
     * Get the detailed information of all the stocks
     */
    virtual vector<StockInfo> getAllStockInfo() = 0;

    /**
     * Get the information of the given security
     * @param market market abbreviation
     * @param code security code
     */
    virtual StockInfo getStockInfo(string market, const string& code) = 0;

    /**
     * Get the ex-rights/ex-dividend list within the given date range [start, end)
     * @param market market abbreviation
     * @param code security code
     * @param start start date
     * @param end end date
     */
    virtual StockWeightList getStockWeightList(const string& market, const string& code,
                                               Datetime start, Datetime end);

    virtual unordered_map<string, StockWeightList> getAllStockWeightList() {
        unordered_map<string, StockWeightList> ret;
        return ret;
    }

    /**
     * Get the historical financial information
     * @param market market abbreviation
     * @param code security code
     * @param start start date of the financial report release
     * @param end end date of the query
     * @return vector<float> [[financial report release date (ymd), field1, field2, ...], ...]
     */
    virtual vector<HistoryFinanceInfo> getHistoryFinance(const string& market, const string& code,
                                                         Datetime start, Datetime end) {
        return vector<HistoryFinanceInfo>();
    }

    /**
     * Get the field indexes and names of the historical financial information
     * @return vector<std::pair<size_t, string>>
     */
    virtual vector<std::pair<size_t, string>> getHistoryFinanceField() {
        return vector<std::pair<size_t, string>>();
    }

    /**
     * Get the current financial information
     * @param market market identifier
     * @param code security code
     */
    virtual Parameter getFinanceInfo(const string& market, const string& code);

    /**
     * Get the given MarketInfo
     * @param market market abbreviation
     * @return Null<MarketInfo>() is returned if it is not found
     */
    virtual MarketInfo getMarketInfo(const string& market) = 0;

    /**
     * Get all the market information
     */
    virtual vector<MarketInfo> getAllMarketInfo() = 0;

    /**
     * Get all the security type information
     */
    virtual vector<StockTypeInfo> getAllStockTypeInfo() = 0;

    /**
     * Get the detailed information of the corresponding security type
     * @param type security type
     * @return the corresponding security type information; Null<StockTypeInf>() is returned if it
     *         does not exist
     */
    virtual StockTypeInfo getStockTypeInfo(uint32_t type) = 0;

    /**
     * Get all the holiday dates
     */
    virtual std::unordered_set<Datetime> getAllHolidays() = 0;

    /**
     * Get the All Zh Bond10 object
     * @return ZhBond10List
     */
    virtual ZhBond10List getAllZhBond10() = 0;

    /**
     * Get all the historical financial information, used for the data initialization in column
     * storage
     */
    virtual unordered_map<string, vector<HistoryFinanceInfo>> getAllHistoryFinance(
      const std::atomic_bool& cancel_flag) {
        HKU_ERROR("Not support getAllHistoryFinance()!");
        return unordered_map<string, vector<HistoryFinanceInfo>>();
    }

private:
    bool checkType();

protected:
    string m_name;
};

typedef shared_ptr<BaseInfoDriver> BaseInfoDriverPtr;

HKU_API std::ostream& operator<<(std::ostream&, const BaseInfoDriver&);
HKU_API std::ostream& operator<<(std::ostream&, const BaseInfoDriverPtr&);

inline const string& BaseInfoDriver::name() const {
    return m_name;
}

} /* namespace hku */
#endif /* BASEINFODRIVER_H_ */
