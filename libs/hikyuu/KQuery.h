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

        //BTC扩展
        MIN3     = 11,  ///<3分钟线
        HOUR2    = 12,  ///<2小时线
        HOUR4    = 13,  ///<4小时线
        HOUR6    = 14,  ///<6小时线
        HOUR12   = 15,  ///<12小时线
        INVALID_KTYPE = 16
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
     * K线查询，范围[start, end)
     * @param start 起始索引，支持负数
     * @param end  结束索引（不包含本身），支持负数
     * @param dataType K线类型
     * @param recoverType 复权类型
     * @param queryType 默认按索引方式查询
     */
    KQuery(hku_int64 start,
           hku_int64 end = Null<hku_int64>(),
           KType dataType = DAY,
           RecoverType recoverType = NO_RECOVER,
           QueryType queryType = INDEX)
    : m_start(start),
      m_end(end),
      m_queryType(queryType),
      m_dataType(dataType),
      m_recoverType(recoverType) { }

    /**
     * 按索引方式查询时，返回指定的起始索引，否则返回Null<hku_int64>()
     */
    hku_int64 start() const {
        return m_queryType != INDEX ? Null<hku_int64>() : m_start;
    }

    /**
     * 按索引方式查询时，返回指定的结束索引，否则返回Null<hku_int64>()
     */
    hku_int64 end() const {
        return m_queryType != INDEX ? Null<hku_int64>() : m_end;
    }

    /**
     * 按日期方式查询时，返回指定的起始日期，否则返回Null<Datetime>()
     */
    Datetime startDatetime() const {
        return m_queryType != DATE
                ? Null<Datetime>()
                : Datetime((hku_uint64)m_start);
    }

    /**
     * 按日期方式查询时，返回指定的结束日期，否则返回Null<Datetime>()
     */
    Datetime endDatetime() const {
        return m_queryType != DATE
                ? Null<Datetime>()
                : Datetime((hku_uint64)m_end);
    }

    /** 获取查询条件类型 */
    QueryType queryType() const { return m_queryType; }

    /** 获取K线数据类型 */
    KType kType() const { return m_dataType; }

    /** 获取复权类型 */
    RecoverType recoverType() const { return m_recoverType; }

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

private:
    hku_int64 m_start;
    hku_int64 m_end;
    QueryType m_queryType;
    KType m_dataType;
    RecoverType m_recoverType;
};


/**
 * 构造按索引方式K线查询，范围[start, end)
 * @param start 起始索引，支持负数
 * @param end  结束索引（不包含本身），支持负数
 * @param dataType K线类型
 * @param recoverType 复权类型
 * @see KQuery
 * @ingroup StockManage*
 */
inline KQuery KQueryByIndex(hku_int64 start = 0,
        hku_int64 end = Null<hku_int64>(),
        KQuery::KType dataType = KQuery::DAY,
        KQuery::RecoverType recoverType = KQuery::NO_RECOVER) {
    return KQuery(start, end, dataType, recoverType, KQuery::INDEX);
}

/**
 * 构造按日期方式K线查询，范围[startDatetime, endDatetime)
 * @param start 起始日期
 * @param end  结束日期（不包含本身）
 * @param dataType K线类型
 * @param recoverType 复权类型
 * @see KQuery
 * @ingroup StockManage
 */
inline KQuery KQueryByDate(
        const Datetime& start = Datetime::min(),
        const Datetime& end = Null<Datetime>(),
        KQuery::KType dataType = KQuery::DAY,
        KQuery::RecoverType recoverType = KQuery::NO_RECOVER) {
    return KQuery((hku_int64)start.number(), (hku_int64)end.number(),
                  dataType, recoverType, KQuery::DATE);
}

/**
 * 输出KQuery信息，如：KQuery(start, end, queryType, kType, recoverType)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator <<(std::ostream &os, const KQuery& query);


///////////////////////////////////////////////////////////////////////////////
//
// 关系比较函数, 不直接在类中定义是为了支持 Null<>() == d，Null可以放在左边
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const KQuery&, const KQuery&);
bool operator!=(const KQuery&, const KQuery&);

inline bool operator!=(const KQuery& q1, const KQuery& q2) {
    if (q1.start() != q2.start()
            || q1.end() != q2.end()
            || q1.queryType() != q2.queryType()
            || q1.kType() != q2.kType()
            || q1.recoverType() != q2.recoverType()) {
        return true;
    }
    return false;
}

inline bool operator==(const KQuery& q1, const KQuery& q2) {
    if (q1 != q2) {
        return false;
    }
    return true;
}

/**
 * 提供KQuery的Null值
 * @ingroup StockManage
 */
template <>
class Null<KQuery> {
public:
    Null() {}
    operator KQuery() {
        return KQuery(Null<hku_int64>(),
                Null<hku_int64>(),
                KQuery::INVALID_KTYPE,
                KQuery::INVALID_RECOVER_TYPE,
                KQuery::INVALID
                );
    }
};

} /* namespace */

#endif /* KQUERY_H_ */
