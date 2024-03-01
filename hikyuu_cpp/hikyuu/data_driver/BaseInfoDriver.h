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
 * 基本信息数据获取驱动基类
 * @ingroup DataDriver
 */
class HKU_API BaseInfoDriver {
    PARAMETER_SUPPORT

public:
    typedef unordered_map<string, MarketInfo> MarketInfoMap;
    typedef unordered_map<uint32_t, StockTypeInfo> StockTypeInfoMap;

    /**
     * 构造函数
     * @param name 驱动名称
     */
    BaseInfoDriver(const string& name);
    virtual ~BaseInfoDriver() {}

    /** 获取驱动名称 */
    const string& name() const;

    /**
     * 驱动初始化
     * @param params
     * @return
     */
    bool init(const Parameter& params);

    /**
     * 驱动初始化，具体实现时应注意将之前打开的相关资源关闭。
     * @return
     */
    virtual bool _init() = 0;

    /**
     * 获取所有股票详情信息
     */
    virtual vector<StockInfo> getAllStockInfo() = 0;

    /**
     * 获取指定的证券信息
     * @param market 市场简称
     * @param code 证券代码
     */
    virtual StockInfo getStockInfo(string market, const string& code) = 0;

    /**
     * 获取指定日期范围内 [start, end) 的权限列表
     * @param market 市场简称
     * @param code 证券代码
     * @param start 起始日期
     * @param end 结束日期
     */
    virtual StockWeightList getStockWeightList(const string& market, const string& code,
                                               Datetime start, Datetime end);

    virtual unordered_map<string, StockWeightList> getAllStockWeightList() {
        unordered_map<string, StockWeightList> ret;
        return ret;
    }

    /**
     * 获取当前财务信息
     * @param market 市场标识
     * @param code 证券代码
     */
    virtual Parameter getFinanceInfo(const string& market, const string& code);

    /**
     * 获取指定的MarketInfo
     * @param market 市场简称
     * @return 如未找到，则返回 Null<MarketInfo>()
     */
    virtual MarketInfo getMarketInfo(const string& market) = 0;

    /**
     * 获取全部市场信息
     */
    virtual vector<MarketInfo> getAllMarketInfo() = 0;

    /**
     * 获取全部证券类型信息
     */
    virtual vector<StockTypeInfo> getAllStockTypeInfo() = 0;

    /**
     * 获取相应的证券类型详细信息
     * @param type 证券类型
     * @return 对应的证券类型信息，如果不存在，则返回Null<StockTypeInf>()
     */
    virtual StockTypeInfo getStockTypeInfo(uint32_t type) = 0;

    /**
     * 获取所有节假日日期
     */
    virtual std::unordered_set<Datetime> getAllHolidays() = 0;

    /**
     * Get the All Zh Bond10 object
     * @return ZhBond10List
     */
    virtual ZhBond10List getAllZhBond10() = 0;

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
