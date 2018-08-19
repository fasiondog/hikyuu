/*
 * Stock.h
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#ifndef STOCK_H_
#define STOCK_H_

#include "StockWeight.h"
#include "KQuery.h"

namespace hku {

class HKU_API StockManager;
class KDataDriver;
typedef shared_ptr<KDataDriver> KDataDriverPtr;
class HKU_API KData;

/**
 * Stock基类，Application中一般使用StockPtr进行操作
 * @ingroup StockManage
 */
class HKU_API Stock {
    friend class StockManager;

private:
    static const string default_market;
    static const string default_code;
    static const string default_market_code;
    static const string default_name;
    static const hku_uint32 default_type;
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

    Stock(const Stock&);
    Stock(const string& market, const string& code, const string& name);

    Stock(const string& market, const string& code,
          const string& name, hku_uint32 type, bool valid,
          const Datetime& startDate, const Datetime& lastDate);
    Stock(const string& market, const string& code,
          const string& name, hku_uint32 type, bool valid,
          const Datetime& startDate, const Datetime& lastDate,
          price_t tick, price_t tickValue, int precision,
          size_t minTradeNumber, size_t maxTradeNumber);
    virtual ~Stock();
    Stock& operator=(const Stock&);
    bool operator==(const Stock&) const;
    bool operator!=(const Stock&) const;

    /** 获取内部id，一般用于作为map的键值使用，该id实质为m_data的内存地址 */
    hku_uint64 id() const;

    /** 获取所属市场简称，市场简称是市场的唯一标识 */
    const string& market() const;

    /** 获取证券代码 */
    const string& code() const;

    /** 市场简称+证券代码，如: sh000001 */
    const string& market_code() const;

    /** 获取证券名称 */
    const string& name() const;

    /** 获取证券类型 */
    hku_uint32 type() const;

    /** 该证券当前是否有效 */
    bool valid() const;

    /** 获取证券起始日期 */
    Datetime startDatetime() const;

    /** 获取证券最后日期 */
    Datetime lastDatetime() const;

    /** 获取最小跳动量 */
    price_t tick() const;

    /** 最小跳动量价值 */
    price_t tickValue() const;

    /** 每单位价值 = tickValue / tick */
    price_t unit() const;

    /** 获取价格精度 */
    int precision() const;

    /** 获取最小交易数量，同minTradeNumber */
    size_t atom() const;

    /** 获取最小交易数量 */
    size_t minTradeNumber() const;

    /** 获取最大交易量 */
    size_t maxTradeNumber() const;

    /** 获取所有权息信息 */
    StockWeightList getWeight() const;

    /**
     * 获取指定时间段[start,end)内的权息信息
     * @param start 起始日期
     * @param end 结束日期
     * @return 满足要求的权息信息列表指针
     */
    StockWeightList getWeight(const Datetime& start,
                    const Datetime& end = Null<Datetime>()) const;

    /** 获取不同类型K线数据量 */
    size_t getCount(KQuery::KType dataType = KQuery::DAY) const;

    /** 获取指定日期时刻的市值，即小于等于指定日期的最后一条记录的收盘价 */
    price_t getMarketValue(const Datetime&, KQuery::KType) const;

    /**
     * 根据KQuery指定的条件，获取对应的K线位置范围
     * @param query [in] 指定的查询条件
     * @param out_start [out] 对应的K线起始范围
     * @param out_end [out] 对应的K线结束范围，不包含自身
     * @return true 成功 | false 失败
     */
    bool getIndexRange(const KQuery&, size_t& out_start, size_t& out_end) const;

    /** 获取指定索引的K线数据记录，未作越界检查 */
    KRecord getKRecord(size_t pos,
                       KQuery::KType dataType = KQuery::DAY) const;

    /** 根据数据类型（日线/周线等），获取指定日期的KRecord */
    KRecord getKRecordByDate(const Datetime&, KQuery::KType ktype = KQuery::DAY) const;

    /** 获取K线数据 */
    KData getKData(const KQuery&) const;

    /** 获取K线记录，一般不直接使用，用getKData替代 */
    KRecordList getKRecordList(size_t start, size_t end, KQuery::KType) const;

    /** 获取日期列表 */
    DatetimeList getDatetimeList(size_t start, size_t end, KQuery::KType) const;

    /** 获取日期列表 */
    DatetimeList getDatetimeList(const KQuery& query) const;

    /** 设置权息信息 */
    void setWeightList(const StockWeightList&);

    /** 设置K线数据获取驱动 */
    void setKDataDriver(const KDataDriverPtr& kdataDriver);

    /** 获取K线数据获取驱动 */
    KDataDriverPtr getKDataDriver() const;

    /**
     * 将K线数据做自身缓存
     * @note 一般不主动调用，谨慎
     */
    void loadKDataToBuffer(KQuery::KType);

    /** 释放对应的K线缓存 */
    void releaseKDataBuffer(KQuery::KType);

    /** 指定类型的K线数据是否被缓存 */
    bool isBuffer(KQuery::KType) const;

    /** 是否为Null */
    bool isNull() const;

    /** （临时函数）只用于更新缓存中的日线数据 **/
    void realtimeUpdate(const KRecord&);

    /** 仅用于python的__str__ */
    string toString() const;

private:
    bool _getIndexRangeByIndex(const KQuery&, size_t& out_start, size_t& out_end) const;
    bool _getIndexRangeByDateFromBuffer(const KQuery&, size_t&, size_t&) const;

private:
    struct HKU_API Data;
    shared_ptr<Data> m_data;
    KDataDriverPtr m_kdataDriver;
};

struct HKU_API Stock::Data {
    string        m_market;      //所属的市场简称
    string        m_code;        //证券代码
    string        m_market_code;//市场简称证券代码
    string        m_name;        //证券名称
    hku_uint32    m_type;        //证券类型
    bool          m_valid;       //当前证券是否有效
    Datetime      m_startDate;   //证券起始日期
    Datetime      m_lastDate;    //证券最后日期

    StockWeightList m_weightList; //权息信息列表

    price_t m_tick;
    price_t m_tickValue;
    price_t m_unit;
    int     m_precision;
    size_t  m_minTradeNumber;
    size_t  m_maxTradeNumber;

    KRecordListPtr pKData[KQuery::INVALID_KTYPE];

    Data();
    Data(const string& market, const string& code,
          const string& name, hku_uint32 type, bool valid,
          const Datetime& startDate, const Datetime& lastDate,
          price_t tick, price_t tickValue, int precision,
          size_t minTradeNumber, size_t maxTradeNumber);

    virtual ~Data();
};

/**
 * 输出Stock信息，如：Stock(market, code, name, type, valid, startDatetime, lastDatetime)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator <<(std::ostream &os, const Stock& stock);

/** @ingroup StockManage */
typedef vector<Stock> StockList;


/* 用于将Stock实例作为map的key，一般建议使用stock.id做键值，
 * 否则map还要利用拷贝构造函数，创建新对象，效率低 */
bool operator < (const Stock& s1, const Stock& s2);
inline bool operator < (const Stock& s1, const Stock& s2) {
    return s1.id() < s2.id();
}

inline hku_uint64 Stock::id() const {
    return isNull() ? 0 : (hku_int64)m_data.get();
}

inline StockWeightList Stock::getWeight() const {
    return m_data ? m_data->m_weightList : StockWeightList();
}

inline bool Stock::operator==(const Stock& stock) const {
    return (*this != stock) ? false : true;
}

} /* namespace */

#endif /* STOCK_H_ */
