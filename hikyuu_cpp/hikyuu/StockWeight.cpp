/*
 * StockWeight.cpp
 *
 *  Created on: 2011-12-9
 *      Author: fasiondog
 */

#include "StockWeight.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const StockWeight& record) {
    if (Null<StockWeight>() == record) {
        os << "Weight(Null)";
        return os;
    }

    os << "Weight(" << record.datetime() << ", " << record.countAsGift() << ", "
       << record.countForSell() << ", " << record.priceForSell() << ", " << record.bonus() << ", "
       << record.increasement() << ", " << record.totalCount() << ", " << record.freeCount() << ", "
       << record.suogu() << ")";
    os.precision(6);
    return os;
}

StockWeight::StockWeight(const Datetime& datetime)
: m_datetime(datetime),
  m_countAsGift(0.0),
  m_countForSell(0.0),
  m_priceForSell(0.0),
  m_bonus(0.0),
  m_increasement(0.0),
  m_totalCount(0.0),
  m_freeCount(0.0),
  m_suogu(0.0) {}

StockWeight::StockWeight(const Datetime& datetime, price_t countAsGift, price_t countForSell,
                         price_t priceForSell, price_t bonus, price_t increasement,
                         price_t totalCount, price_t freeCount, price_t suogu)
: m_datetime(datetime),
  m_countAsGift(countAsGift),
  m_countForSell(countForSell),
  m_priceForSell(priceForSell),
  m_bonus(bonus),
  m_increasement(increasement),
  m_totalCount(totalCount),
  m_freeCount(freeCount),
  m_suogu(suogu) {}

}  // namespace hku
