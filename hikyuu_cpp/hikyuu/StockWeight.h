/*
 * StockWeight.h
 *
 *  Created on: 2011-12-9
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCKWEIGHT_H_
#define STOCKWEIGHT_H_

#include "DataType.h"

namespace hku {

/**
 * 权息数据结构
 * @ingroup StockManage
 */
class HKU_API StockWeight {
public:
    /** 默认构造函数，返回Null<StockWeight>() */
    StockWeight();

    explicit StockWeight(const Datetime& datetime);

    StockWeight(const Datetime& datetime, price_t countAsGift, price_t countForSell,
                price_t priceForSell, price_t bonus, price_t increasement, price_t totalCount,
                price_t freeCount);

    /** 权息日期 */
    Datetime datetime() const {
        return m_datetime;
    }

    /** 每10股送X股 */
    price_t countAsGift() const {
        return m_countAsGift;
    }

    /** 每10股配X股 */
    price_t countForSell() const {
        return m_countForSell;
    }

    /** 配股价 */
    price_t priceForSell() const {
        return m_priceForSell;
    }

    /** 每10股红利 */
    price_t bonus() const {
        return m_bonus;
    }

    /** 每10股转增X股 */
    price_t increasement() const {
        return m_increasement;
    }

    /** 总股本（万股） */
    price_t totalCount() const {
        return m_totalCount;
    }

    /** 流通股（万股） */
    price_t freeCount() const {
        return m_freeCount;
    }

private:
    Datetime m_datetime;     // 权息日期
    price_t m_countAsGift;   // 每10股送X股
    price_t m_countForSell;  // 每10股配X股
    price_t m_priceForSell;  // 配股价
    price_t m_bonus;         // 每10股红利
    price_t m_increasement;  // 每10股转增X股
    price_t m_totalCount;    // 总股本（万股）
    price_t m_freeCount;     // 流通股（万股）
};

/** @ingroup StockManage */
typedef vector<StockWeight> StockWeightList;

/**
 * 输出权息信息，如：Weight(datetime, countAsGift, countForSell,
 * priceForSell, bonus, increasement, totalCount, freeCount)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const StockWeight&);

///////////////////////////////////////////////////////////////////////////////
//
// 关系比较函数
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const StockWeight&, const StockWeight&);
bool operator!=(const StockWeight&, const StockWeight&);
bool operator>(const StockWeight&, const StockWeight&);
bool operator<(const StockWeight&, const StockWeight&);
bool operator>=(const StockWeight&, const StockWeight&);
bool operator<=(const StockWeight&, const StockWeight&);

/* 相等比较, 仅根据日期判断 */
inline bool operator==(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() == m2.datetime();
}

/* 不等比较， 仅根据日期判断 */
inline bool operator!=(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() != m2.datetime();
}

/* 大于比较, 仅根据日期判断 */
inline bool operator>(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() > m2.datetime();
}

/* 小于比较， 仅根据日期判断 */
inline bool operator<(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() < m2.datetime();
}

/* 大于等于比较, 仅根据日期判断 */
inline bool operator>=(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() >= m2.datetime();
}

/* 小于等于比较， 仅根据日期判断 */
inline bool operator<=(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() <= m2.datetime();
}

/** @} */
}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::StockWeight> : ostream_formatter {};
#endif

#endif /* STOCKWEIGHT_H_ */
