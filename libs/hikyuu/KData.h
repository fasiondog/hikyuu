/*
 * KData.h
 *
 *  Created on: 2012-9-25
 *      Author: fasiondog
 */

#ifndef KDATA_H_
#define KDATA_H_

#include "KDataImp.h"

namespace hku {

/**
 * K线数据
 * @ingroup StockManage
 */
class HKU_API KData {
public:
    KData(){}
    KData(const KData&);
    KData(const Stock& stock, const KQuery& query);
    virtual ~KData() { }

    KData& operator=(const KData&);

    size_t size() const;
    bool empty() const;

    DatetimeList getDatetimeList() const;

    /** 获取指定位置的KRecord，未作越界检查 */
    KRecord getKRecord(size_t pos) const;

    /** 按日期查询KRecord */
    KRecord getKRecordByDate(const Datetime& datetime) const;

    /** 同getKRecord @see getKRecord */
    KRecord operator[](size_t pos) const { return getKRecord(pos); }

    /** 同getKRecordByDate @see getKRecordByDate */
    KRecord operator[](const Datetime& datetime) const {
        return getKRecordByDate(datetime);
    }

    /** 按日期查询对应的索引位置  */
    size_t getPos(const Datetime& datetime) const;

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
HKU_API std::ostream& operator <<(std::ostream &os, const KData& kdata);


inline KData::KData(const KData& x) {
    m_imp = x.m_imp;
}


inline KData& KData::operator=(const KData& x) {
    if(this == &x)
        return *this;

    if (m_imp != x.m_imp)
        m_imp = x.m_imp;

    return *this;
}


inline DatetimeList KData::getDatetimeList() const {
    if (empty()) {
        return DatetimeList();
    }
    return getStock().getDatetimeList(startPos(), lastPos() + 1,
                                      getQuery().kType());
}


inline KRecord KData::getKRecord(size_t pos) const {
    return m_imp->getKRecord(pos); //如果为空，将抛出异常
}


inline KRecord KData::getKRecordByDate(const Datetime& datetime) const {
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

} /* namespace hku */
#endif /* KDATA_H_ */
