/*
 * KData.h
 *
 *  Created on: 2012-9-25
 *      Author: fasiondog
 */

#pragma once
#ifndef KDATA_H_
#define KDATA_H_

#include "Stock.h"

namespace hku {

class HKU_API Indicator;

/**
 * K线数据
 * @ingroup StockManage
 */
class HKU_API KData {
public:
    KData() {}
    KData(const KData&);
    KData(KData&&);
    KData(const Stock& stock, const KQuery& query);
    virtual ~KData() {}

    KData& operator=(const KData&);
    KData& operator=(KData&&);

    size_t size() const;
    bool empty() const;

    bool operator==(const KData&);

    DatetimeList getDatetimeList() const;

    /** 获取指定位置的KRecord，未作越界检查 */
    KRecord getKRecord(size_t pos) const;

    /** 按日期查询KRecord */
    KRecord getKRecord(Datetime datetime) const;

    /** 同getKRecord @see getKRecord */
    KRecord operator[](size_t pos) const {
        return m_buffer[pos];
    }

    /** 同getKRecord @see getKRecord */
    KRecord operator[](Datetime datetime) const {
        return getKRecord(datetime);
    }

    /** 按日期查询对应的索引位置，注：是 KData 中的位置，不是在 Stock 中原始K记录的位置 */
    size_t getPos(const Datetime& datetime) const;

    /** 按日期获取在原始 K 线记录中的位置 */
    size_t getPosInStock(Datetime datetime) const;

    /** 获取关联的KQuery */
    KQuery getQuery() const;

    /** 获取关联的Stock，如果没有关联返回Null<Stock> */
    Stock getStock() const;

    /** 获取在原始K线记录中对应的起始位置，如果为空返回0 */
    size_t startPos() const;

    /** 获取在原始K线记录中对应的最后一条记录的位置，如果为空返回0,其他等于endPos - 1 */
    size_t lastPos() const;

    /** 获取在原始K线记录中对应范围的下一条记录的位置，如果为空返回0,其他等于lastPos + 1 */
    size_t endPos() const;

    size_t expand(size_t num);

    /** 输出数据到指定的文件中 */
    void tocsv(const string& filename);

    string toString() const;

    /** 开盘价 */
    Indicator open() const;

    /** 最高价 */
    Indicator high() const;

    /** 收盘价 */
    Indicator close() const;

    /** 最低价 */
    Indicator low() const;

    /** 成交量 */
    Indicator vol() const;

    /** 成交金额 */
    Indicator amo() const;

private:
    void _recoverForward();
    void _recoverBackward();
    void _recoverEqualForward();
    void _recoverEqualBackward();
    void _recoverForUpDay();

private:
    KRecordList m_buffer;
    KQuery m_query;
    Stock m_stock;
    size_t m_start;
    size_t m_end;
};

/**
 * 输出KData信息
 * @details
 * <pre>
 * KData{
 *   size : 738501
 *   stock: Stock(SH, 000001, 上证指数, 指数, 1, 1990-Dec-19 00:00:00, +infinity),
 *   query: KQuery(0, 99999999999, INDEX, MIN, NO_RECOVER)
 *  }
 * </pre>
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream& os, const KData& kdata);

/**
 * 根据股票标识按指定的查询条件查询的 K 线数据
 * @param market_code 股票标识
 * @param query 查询条件
 * @ingroup StockManage
 */
KData HKU_API getKData(const string& market_code, const KQuery& query);

/**
 * 根据股票标识直接按日期查询获取相应的 K 线数据
 * @param market_code 股票标识
 * @param start 起始日期
 * @param end 结束日期
 * @param ktype K线类型
 * @param recoverType 复权类型
 * @ingroup StockManage
 */
KData HKU_API getKData(const string& market_code, const Datetime& start = Datetime::min(),
                       const Datetime& end = Null<Datetime>(), KQuery::KType ktype = KQuery::DAY,
                       KQuery::RecoverType recoverType = KQuery::NO_RECOVER);

/**
 * 根据股票标识直接按索引位置查询获取相应的 K 线数据
 * @param market_code 股票标识
 * @param start 起始索引
 * @param end 结束索引
 * @param ktype K线类型
 * @param recoverType 复权类型
 * @ingroup StockManage
 */
KData HKU_API getKData(const string& market_code, int64_t start = 0, int64_t end = Null<int64_t>(),
                       KQuery::KType ktype = KQuery::DAY,
                       KQuery::RecoverType recoverType = KQuery::NO_RECOVER);

inline KRecord KData::getKRecord(size_t pos) const {
    return m_buffer[pos];
}

inline KRecord KData::getKRecord(Datetime datetime) const {
    size_t pos = getPos(datetime);
    return pos != Null<size_t>() ? getKRecord(pos) : Null<KRecord>();
}

inline size_t KData::size() const {
    return m_buffer.size();
}

inline bool KData::empty() const {
    return m_buffer.empty();
}

inline KQuery KData::getQuery() const {
    return m_query;
}

inline Stock KData::getStock() const {
    return m_stock;
}

inline size_t KData::startPos() const {
    return m_start;
}

inline size_t KData::endPos() const {
    return m_end;
}

inline size_t KData::lastPos() const {
    return m_end == 0 ? 0 : m_end - 1;
}

} /* namespace hku */
#endif /* KDATA_H_ */
