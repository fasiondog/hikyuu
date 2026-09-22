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
 * Ex-rights/ex-dividend data structure
 * @ingroup StockManage
 */
class HKU_API StockWeight {
public:
    /** Default constructor, returns Null<StockWeight>() */
    StockWeight() = default;

    explicit StockWeight(const Datetime& datetime);

    StockWeight(const Datetime& datetime, price_t countAsGift, price_t countForSell,
                price_t priceForSell, price_t bonus, price_t increasement, price_t totalCount,
                price_t freeCount, price_t suogu);

    /** Ex-rights/ex-dividend date */
    Datetime datetime() const noexcept {
        return m_datetime;
    }

    /** Bonus shares per 10 shares (X shares given per 10 shares) */
    price_t countAsGift() const noexcept {
        return m_countAsGift;
    }

    /** Rights shares per 10 shares (X shares allotted per 10 shares) */
    price_t countForSell() const noexcept {
        return m_countForSell;
    }

    /** Rights issue price */
    price_t priceForSell() const noexcept {
        return m_priceForSell;
    }

    /** Dividend per 10 shares */
    price_t bonus() const noexcept {
        return m_bonus;
    }

    /** Capitalized shares per 10 shares (X shares converted per 10 shares) */
    price_t increasement() const noexcept {
        return m_increasement;
    }

    /** Total share capital (in units of 10 thousand shares) */
    price_t totalCount() const noexcept {
        return m_totalCount;
    }

    /** Outstanding shares (in units of 10 thousand shares) */
    price_t freeCount() const noexcept {
        return m_freeCount;
    }

    /** Share expansion/contraction ratio (suogu) */
    price_t suogu() const noexcept {
        return m_suogu;
    }

private:
    Datetime m_datetime;         // Ex-rights/ex-dividend date
    price_t m_countAsGift{0.};   // Bonus shares per 10 shares
    price_t m_countForSell{0.};  // Rights shares per 10 shares
    price_t m_priceForSell{0.};  // Rights issue price
    price_t m_bonus{0.};         // Dividend per 10 shares
    price_t m_increasement{0.};  // Capitalized shares per 10 shares
    price_t m_totalCount{0.};    // Total share capital (10 thousand shares)
    price_t m_freeCount{0.};     // Outstanding shares (10 thousand shares)
    price_t m_suogu{0.};         // Share expansion/contraction ratio
};

/** @ingroup StockManage */
typedef vector<StockWeight> StockWeightList;

/**
 * Output the ex-rights/ex-dividend information, e.g.: Weight(datetime, countAsGift, countForSell,
 * priceForSell, bonus, increasement, totalCount, freeCount)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const StockWeight&);

///////////////////////////////////////////////////////////////////////////////
//
// Relational comparison functions
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const StockWeight&, const StockWeight&);
bool operator!=(const StockWeight&, const StockWeight&);
bool operator>(const StockWeight&, const StockWeight&);
bool operator<(const StockWeight&, const StockWeight&);
bool operator>=(const StockWeight&, const StockWeight&);
bool operator<=(const StockWeight&, const StockWeight&);

/* Equal comparison, judged by the date only */
inline bool operator==(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() == m2.datetime();
}

/* Unequal comparison, judged by the date only */
inline bool operator!=(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() != m2.datetime();
}

/* Greater-than comparison, judged by the date only */
inline bool operator>(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() > m2.datetime();
}

/* Less-than comparison, judged by the date only */
inline bool operator<(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() < m2.datetime();
}

/* Greater-than-or-equal comparison, judged by the date only */
inline bool operator>=(const StockWeight& m1, const StockWeight& m2) {
    return m1.datetime() >= m2.datetime();
}

/* Less-than-or-equal comparison, judged by the date only */
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
