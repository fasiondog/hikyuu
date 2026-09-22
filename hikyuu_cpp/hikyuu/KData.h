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
 * K-line (candlestick) data
 * @ingroup StockManage
 */
class HKU_API KData {
public:
    KData();
    KData(const KData&) noexcept;
    KData(const Stock& stock, const KQuery& query);
    explicit KData(KDataImpPtr imp);
    virtual ~KData() {}

    KData& operator=(const KData&) noexcept;

    // Move semantics brings no real benefit to KData, and it may leave KData with a null imp.
    // They are kept mainly because boost::any_cast needs them, but do not std::move a KData.
    KData(KData&&);
    KData& operator=(KData&&);

    size_t size() const noexcept;
    bool empty() const noexcept;

    bool operator==(const KData&) const noexcept;
    bool operator!=(const KData&) const noexcept;

    DatetimeList getDatetimeList() const;

    /** Get the KRecord at the given position, no bounds check is performed */
    const KRecord& getKRecord(size_t pos) const noexcept;

    /** Query the KRecord by date */
    const KRecord& getKRecord(Datetime datetime) const noexcept;

    /** Same as getKRecord @see getKRecord */
    const KRecord& operator[](size_t pos) const noexcept {
        return getKRecord(pos);
    }

    /** Same as getKRecord @see getKRecord */
    const KRecord& operator[](Datetime datetime) const {
        return getKRecord(datetime);
    }

    const KRecord& front() const;
    const KRecord& back() const;

    /**
     * Get a new KData from the current one, keeping the data type and price adjustment type
     * unchanged
     * @note The new KData is not necessarily a subset of the original one
     * @param start start date
     * @param end end date
     */
    KData getKData(const Datetime& start, const Datetime& end) const;

    KData getKData(const KQuery& query) const;

    /**
     * Get the K-line data of another type over the same time range, e.g. the minute data
     * corresponding to the daily data
     * @param ktype
     * @return KData
     */
    KData getKData(const KQuery::KType& ktype) const;

    /**
     * Get the subset [start, end) by index
     * @param start start index
     * @param end end index
     * @return KData
     */
    KData getSubKData(int64_t start, int64_t end = Null<int64_t>()) const;

    /** Get the index position of the given date. Note: it is the position inside this KData, not
     * the position of the original K-line record in the Stock */
    size_t getPos(const Datetime& datetime) const noexcept;

    /** Get the position in the original K-line record by date */
    size_t getPosInStock(Datetime datetime) const;

    /** Get the associated KQuery */
    const KQuery& getQuery() const;

    /** Get the associated Stock; Null<Stock> is returned if there is no association */
    const Stock& getStock() const;

    /** Get the start position in the original K-line record; 0 if it is empty */
    size_t startPos() const;

    /** Get the position of the last record in the original K-line record; 0 if it is empty,
     *  otherwise endPos - 1 */
    size_t lastPos() const;

    /** Get the position of the next record after the range in the original K-line record; 0 if it
     * is empty, otherwise lastPos + 1 */
    size_t endPos() const;

    /** Write the data to the given file */
    void tocsv(const string& filename);

    string toString() const;

    /** Open price */
    Indicator open() const;

    /** High price */
    Indicator high() const;

    /** Close price */
    Indicator close() const;

    /** Low price */
    Indicator low() const;

    /** Trading volume */
    Indicator vol() const;

    /** Trading amount */
    Indicator amo() const;

    /**
     * Special purpose! Use with care! Get a query condition by date for another K-line type over
     * the given date range, based on the current K-line range
     * @note
     *  1. The given date range must fall inside the current K-line data range, otherwise it is
     *     truncated to the K-line range
     *  2. The precision of start_datetime / end_datetime should match the current KData
     *  3. If the original end condition is Null<Datetime>() and end_datetime is not given, the
     *     returned query condition is Null<Datetime>()
     * @param start_datetime
     * @param end_datetime
     * @param ktype
     * @return KData
     */
    KQuery getOtherQueryByDate(const Datetime& start_datetime, const Datetime& end_datetime,
                               const KQuery::KType& ktype) const;

public:
    const KRecord* data() const noexcept;
    KRecord* data() noexcept;  // Use with care (intended for forcibly adjusting the data)

    KDataImpPtr getImp() const noexcept;

    // Constant iterator definition
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
        const KData& container_;  // Const reference to the container
        size_t index_;            // Current index
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
    std::shared_ptr<KDataImp>& get_null_kdata_imp() {
        static std::shared_ptr<KDataImp> instance =
          std::make_shared<KDataImp>();  // Initialized on the first call
        return instance;
    }

private:
    KDataImpPtr m_imp;
};

/**
 * Print the KData information
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
 * Get the K-line data of the given security identifier with the given query condition
 * @param market_code security identifier
 * @param query query condition
 * @ingroup StockManage
 */
KData HKU_API getKData(const string& market_code, const KQuery& query);

/**
 * Get the K-line data of the given security identifier directly by date
 * @param market_code security identifier
 * @param start start date
 * @param end end date
 * @param ktype K-line type
 * @param recoverType price adjustment type
 * @ingroup StockManage
 */
KData HKU_API getKData(const string& market_code, const Datetime& start = Datetime::min(),
                       const Datetime& end = Null<Datetime>(),
                       const KQuery::KType& ktype = KQuery::DAY,
                       KQuery::RecoverType recoverType = KQuery::NO_RECOVER);

/**
 * Get the K-line data of the given security identifier directly by index position
 * @param market_code security identifier
 * @param start start index
 * @param end end index
 * @param ktype K-line type
 * @param recoverType price adjustment type
 * @ingroup StockManage
 */
KData HKU_API getKData(const string& market_code, int64_t start = 0, int64_t end = Null<int64_t>(),
                       const KQuery::KType& ktype = KQuery::DAY,
                       KQuery::RecoverType recoverType = KQuery::NO_RECOVER);

inline KData::KData(const KData& x) noexcept : m_imp(x.m_imp) {}

inline KData::KData(KData&& x) : m_imp(std::move(x.m_imp)) {
    x.m_imp = get_null_kdata_imp();
}

inline KData& KData::operator=(const KData& x) noexcept {
    HKU_IF_RETURN(this == &x, *this);
    m_imp = x.m_imp;
    return *this;
}

inline KData& KData::operator=(KData&& x) {
    HKU_IF_RETURN(this == &x, *this);
    m_imp = std::move(x.m_imp);
    x.m_imp = get_null_kdata_imp();
    return *this;
}

inline DatetimeList KData::getDatetimeList() const {
    return m_imp->getDatetimeList();
}

inline const KRecord& KData::getKRecord(size_t pos) const noexcept {
    return m_imp->getKRecord(pos);  // Never throws
}

inline const KRecord& KData::getKRecord(Datetime datetime) const noexcept {
    size_t pos = getPos(datetime);
    return pos != Null<size_t>() ? getKRecord(pos) : KRecord::NullKRecord;
}

inline size_t KData::getPos(const Datetime& datetime) const noexcept {
    return m_imp->getPos(datetime);
}

inline size_t KData::size() const noexcept {
    return m_imp->size();
}

inline bool KData::empty() const noexcept {
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

inline bool KData::operator!=(const KData& other) const noexcept {
    return !(*this == other);
}

inline const KRecord* KData::data() const noexcept {
    return m_imp->data();
}

inline KRecord* KData::data() noexcept {
    return m_imp->data();
}

inline KDataImpPtr KData::getImp() const noexcept {
    return m_imp;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::KData> : ostream_formatter {};
#endif

#endif /* KDATA_H_ */
