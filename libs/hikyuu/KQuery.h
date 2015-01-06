/*
 * KQuery.h
 *
 *  Created on: 2009-11-23
 *      Author: fasiondog
 */

#ifndef KQUERY_H_
#define KQUERY_H_

#include "KRecord.h"

namespace hku {

/**
 * 按索引方式查询K线数据条件
 * @ingroup StockManage
 */
class HKU_API KQuery {
public:
    ///查询方式：索引或日期
    enum QueryType {
        INDEX = 0,  ///<按索引方式查询
        DATE  = 1,  ///<按日期方式查询
        INVALID = 3
    };

    ///查询K线类型：日线/周线等
    enum KType {
        //notes: 如添加新类型，请注意按大小顺序添加，否则可能出错
        MIN      = 0,  ///<1分钟线
        MIN5     = 1,  ///<5分钟线
        MIN15    = 2,  ///<15分钟线
        MIN30    = 3,  ///<30分钟线
        MIN60    = 4,  ///<60分钟线
        DAY      = 5,  ///<日线
        WEEK     = 6,  ///<周线
        MONTH    = 7,  ///<月线
        QUARTER  = 8,  ///<季线
        HALFYEAR = 9,  ///<半年线
        YEAR     = 10, ///<年线
        INVALID_KTYPE = 11
    };

    /**
     * 复权类型
     * @note 日线以上，如周线/月线不支持复权
     */
    enum RecoverType {
        NO_RECOVER = 0,           ///<不复权
        FORWARD = 1,              ///<前向复权
        BACKWARD = 2,             ///<后向复权
        EQUAL_FORWARD = 3,        ///<等比前向复权
        EQUAL_BACKWARD = 4,       ///<等比后向复权
        INVALID_RECOVER_TYPE = 5
    };

    /** 默认构造，按索引方式查询全部日线数据，不复权 */
    KQuery()
    : m_start(0),
      m_end(Null<hku_int64>()),
      m_queryType(INDEX),
      m_dataType(DAY),
      m_recoverType(NO_RECOVER) { };

    /**
     * 构造按索引方式K线查询，范围[start, end)
     * @param start 起始索引，支持负数
     * @param end  结束索引（不包含本身），支持负数
     * @param dataType K线类型
     * @param recoverType 复权类型
     */
    KQuery(hku_int64 start,
           hku_int64 end = Null<hku_int64>(),
           KType dataType = DAY,
           RecoverType recoverType = NO_RECOVER)
    : m_start(start),
      m_end(end),
      m_queryType(INDEX),
      m_dataType(dataType),
      m_recoverType(recoverType) { }

    /**
     * 按索引方式查询时，返回指定的起始索引，否则返回Null<hku_int64>()
     */
    const hku_int64 start() const {
        return m_queryType != INDEX ? Null<hku_int64>() : m_start;
    }

    /**
     * 按索引方式查询时，返回指定的结束索引，否则返回Null<hku_int64>()
     */
    const hku_int64 end() const {
        return m_queryType != INDEX ? Null<hku_int64>() : m_end;
    }

    /**
     * 按日期方式查询时，返回指定的起始日期，否则返回Null<Datetime>()
     */
    const Datetime startDatetime() const {
        return m_queryType != DATE
                ? Null<Datetime>()
                : Datetime((hku_uint64)m_start);
    }

    /**
     * 按日期方式查询时，返回指定的结束日期，否则返回Null<Datetime>()
     */
    const Datetime endDatetime() const {
        return m_queryType != DATE
                ? Null<Datetime>()
                : Datetime((hku_uint64)m_end);
    }

    /** 获取查询条件类型 */
    const QueryType queryType() const { return m_queryType; }

    /** 获取K线数据类型 */
    const KType kType() const { return m_dataType; }

    /** 获取复权类型 */
    const RecoverType recoverType() const { return m_recoverType; }

    /** 获取queryType名称，用于显示输出 */
    static string getQueryTypeName(QueryType);

    /** 获取KType名称，用于显示输出 */
    static string getKTypeName(KType);

    /** 获取recoverType名称，用于显示输出 */
    static string getRecoverTypeName(RecoverType);

    /** 根据字符串名称获取相应的queryType枚举值 */
    static QueryType getQueryTypeEnum(const string&);

    /** 根据字符串名称，获取相应的枚举值 */
    static KType getKTypeEnum(const string&);

    /** 根据字符串名称，获取相应的枚举值 */
    static RecoverType getRecoverTypeEnum(const string&);

protected:
    hku_int64 m_start;
    hku_int64 m_end;
    QueryType m_queryType;
    KType m_dataType;
    RecoverType m_recoverType;
};


/**
 * 按日期查询K线数据条件
 * @ingroup StockManage
 */
class HKU_API KQueryByDate: public KQuery {
public:
    /**
     * 默认构造按日期范围查询全部日线数据，不复权
     */
    KQueryByDate(): KQuery() {
        m_queryType = DATE;
        m_start = (hku_int64)Datetime::minDatetime().number();
        Datetime d = Null<Datetime>();
        m_end = (hku_int64)d.number();
    }

    /**
     * 构造按日期方式K线查询，范围[startDatetime, endDatetime)
     * @param start 起始日期
     * @param end  结束日期（不包含本身）
     * @param dataType K线类型
     * @param recoverType 复权类型
     */
    KQueryByDate(const Datetime& start,
                 const Datetime end = Null<Datetime>(),
                 KType dataType = DAY,
                 RecoverType recoverType = NO_RECOVER)
    : KQuery() {
        m_queryType = DATE;
        m_start = (hku_int64)start.number();
        m_end = (hku_int64)end.number();
        m_dataType = dataType;
        m_recoverType = recoverType;
    }
};

/**
 * 按索引方式查询K线数据，同KQuery
 * @see KQuery
 * @ingroup StockManage
 */
typedef KQuery KQueryByIndex;

/**
 * 输出KQuery信息，如：KQuery(start, end, queryType, kType, recoverType)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator <<(std::ostream &os, const KQuery& query);

/**
 * 输出KQueryByDate信息，如：KQueryByDate(startDatetime, endDatetime,
 * queryType, kType, recoverType)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator <<(std::ostream &os, const KQueryByDate& query);

} /* namespace */

#endif /* KQUERY_H_ */
