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
    KData();
    KData(const KData&);
    KData(const Stock& stock, const KQuery& query);
    virtual ~KData() {}

    KData& operator=(const KData&);

    // 移动语义对 KData 没有实际用处，而且会导致 KData 可能存在空 imp 的情况
    // 主要是 boost::any_cast 需要，予以保留，但使用时不要到 KData 执行 std::move
    KData(KData&&);
    KData& operator=(KData&&);

    size_t size() const;
    bool empty() const;

    bool operator==(const KData&) const;
    bool operator!=(const KData&) const;

    DatetimeList getDatetimeList() const;

    /** 获取指定位置的KRecord，未作越界检查 */
    const KRecord& getKRecord(size_t pos) const;

    /** 按日期查询KRecord */
    const KRecord& getKRecord(Datetime datetime) const;

    /** 同getKRecord @see getKRecord */
    const KRecord& operator[](size_t pos) const {
        return getKRecord(pos);
    }

    /** 同getKRecord @see getKRecord */
    const KRecord& operator[](Datetime datetime) const {
        return getKRecord(datetime);
    }

    const KRecord& front() const;
    const KRecord& back() const;

    /**
     * 通过当前 KData 获取一个保持数据类型、复权类型不变的新的 KData
     * @note 新的 KData 并不一定是原 KData 的子集
     * @param start 起始日期
     * @param end 结束日期
     */
    KData getKData(const Datetime& start, const Datetime& end) const;

    /**
     * 获取相同时间范围内的其他类型K线数据，如日线下对应的分钟线数据
     * @param ktype
     * @return KData
     */
    KData getKData(const KQuery::KType& ktype) const;

    /**
     * 通过索引获取其子集 [star, end)
     * @param start 起始索引
     * @param end 结束索引
     * @return KData
     */
    KData getKData(int64_t start, int64_t end = Null<int64_t>()) const;

    /** 按日期查询对应的索引位置，注：是 KData 中的位置，不是在 Stock 中原始K记录的位置 */
    size_t getPos(const Datetime& datetime) const;

    /** 按日期获取在原始 K 线记录中的位置 */
    size_t getPosInStock(Datetime datetime) const;

    /** 获取关联的KQuery */
    const KQuery& getQuery() const;

    /** 获取关联的Stock，如果没有关联返回Null<Stock> */
    const Stock& getStock() const;

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

public:
    const KRecord* data() const;
    KRecord* data();  // 谨慎使用（用于强制调整数据）

    // 常量迭代器定义
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const KRecord;
        using difference_type = std::ptrdiff_t;
        using pointer = const KRecord*;
        using reference = const KRecord&;

        const_iterator(const KData& container, size_t index)
        : container_(container), index_(index) {}

        reference operator*() const {
            return container_[index_];
        }
        pointer operator->() const {
            return &(container_[index_]);
        }

        const_iterator& operator++() {
            ++index_;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++index_;
            return temp;
        }

        bool operator==(const const_iterator& other) const {
            return &container_ == &other.container_ && index_ == other.index_;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        const KData& container_;  // 常量引用容器
        size_t index_;            // 当前索引
    };

    using iterator = const_iterator;

    const_iterator begin() const {
        return const_iterator(*this, 0);
    }
    const_iterator end() const {
        return const_iterator(*this, size());
    }

    const_iterator cbegin() const {
        return const_iterator(*this, 0);
    }
    const_iterator cend() const {
        return const_iterator(*this, size());
    }

private:
    static shared_ptr<KDataImp> ms_null_kdata_imp;

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

inline KData::KData(KData&& x) : m_imp(std::move(x.m_imp)) {
    x.m_imp = ms_null_kdata_imp;
}

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
    x.m_imp = ms_null_kdata_imp;
    return *this;
}

inline DatetimeList KData::getDatetimeList() const {
    return m_imp->getDatetimeList();
}

inline const KRecord& KData::getKRecord(size_t pos) const {
    return m_imp->getKRecord(pos);  // 不会抛出异常
}

inline const KRecord& KData::getKRecord(Datetime datetime) const {
    size_t pos = getPos(datetime);
    return pos != Null<size_t>() ? getKRecord(pos) : KRecord::NullKRecord;
}

inline size_t KData::getPos(const Datetime& datetime) const {
    return m_imp->getPos(datetime);
}

inline size_t KData::size() const {
    return m_imp->size();
}

inline bool KData::empty() const {
    return m_imp->empty();
}

inline const KQuery& KData::getQuery() const {
    return m_imp->getQuery();
}

inline const Stock& KData::getStock() const {
    return m_imp->getStock();
}

inline const KRecord& KData::front() const {
    return m_imp->front();
}

inline const KRecord& KData::back() const {
    return m_imp->back();
}

inline size_t KData::startPos() const {
    return m_imp->startPos();
}

inline size_t KData::endPos() const {
    return m_imp->endPos();
}

inline size_t KData::lastPos() const {
    return m_imp->lastPos();
}

inline bool KData::operator!=(const KData& other) const {
    return !(*this == other);
}

inline const KRecord* KData::data() const {
    return m_imp->data();
}

inline KRecord* KData::data() {
    return m_imp->data();
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::KData> : ostream_formatter {};
#endif

#endif /* KDATA_H_ */
