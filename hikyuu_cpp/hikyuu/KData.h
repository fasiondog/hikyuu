/*
 * KData.h
 *
 *  Created on: 2012-9-25
 *      Author: fasiondog
 */

#pragma once
#ifndef KDATA_H_
#define KDATA_H_

#include "KDataImp.h"

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

    bool operator==(const KData&) const;
    bool operator!=(const KData&) const;

    DatetimeList getDatetimeList() const;

    /** 获取指定位置的KRecord，未作越界检查 */
    KRecord getKRecord(size_t pos) const;

    /** 按日期查询KRecord */
    KRecord getKRecord(Datetime datetime) const;

    /** 同getKRecord @see getKRecord */
    KRecord operator[](size_t pos) const {
        return getKRecord(pos);
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
    KDataImpPtr m_imp;
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
                       const Datetime& end = Null<Datetime>(),
                       const KQuery::KType& ktype = KQuery::DAY,
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
                       const KQuery::KType& ktype = KQuery::DAY,
                       KQuery::RecoverType recoverType = KQuery::NO_RECOVER);

inline KData::KData(const KData& x) : m_imp(x.m_imp) {}

inline KData::KData(KData&& x) : m_imp(std::move(x.m_imp)) {}

inline KData& KData::operator=(const KData& x) {
    if (this == &x)
        return *this;
    m_imp = x.m_imp;
    return *this;
}

inline KData& KData::operator=(KData&& x) {
    if (this == &x)
        return *this;
    m_imp = std::move(x.m_imp);
    return *this;
}

inline DatetimeList KData::getDatetimeList() const {
    DatetimeList result;
    if (empty()) {
        return result;
    }
    result = getStock().getDatetimeList(KQuery(startPos(), lastPos() + 1, getQuery().kType()));
    return result;
}

inline KRecord KData::getKRecord(size_t pos) const {
    return m_imp->getKRecord(pos);  // 如果为空，将抛出异常
}

inline KRecord KData::getKRecord(Datetime datetime) const {
    size_t pos = getPos(datetime);
    return pos != Null<size_t>() ? getKRecord(pos) : Null<KRecord>();
}

inline size_t KData::getPos(const Datetime& datetime) const {
    return m_imp ? m_imp->getPos(datetime) : Null<size_t>();
}

inline size_t KData::size() const {
    return m_imp ? m_imp->size() : 0;
}

inline bool KData::empty() const {
    return m_imp ? m_imp->empty() : true;
}

inline KQuery KData::getQuery() const {
    return m_imp ? m_imp->getQuery() : Null<KQuery>();
}

inline Stock KData::getStock() const {
    return m_imp ? m_imp->getStock() : Null<Stock>();
}

inline size_t KData::startPos() const {
    return m_imp ? m_imp->startPos() : 0;
}

inline size_t KData::endPos() const {
    return m_imp ? m_imp->endPos() : 0;
}

inline size_t KData::lastPos() const {
    return m_imp ? m_imp->lastPos() : 0;
}

inline bool KData::operator!=(const KData& other) const {
    return !(*this == other);
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::KData> : ostream_formatter {};
#endif

#endif /* KDATA_H_ */
