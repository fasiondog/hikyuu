/*
 * TradeRecord.h
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADERECORD_H_
#define TRADERECORD_H_

#include "../StockManager.h"
#include "../trade_sys/system/SystemPart.h"
#include "CostRecord.h"

#include "../serialization/Datetime_serialization.h"
#include "../serialization/Stock_serialization.h"

namespace hku {

/**
 * Business type
 * @ingroup TradeManagerClass
 */
enum BUSINESS {
    BUSINESS_INIT = 0,           /**< Create the initial account */
    BUSINESS_BUY = 1,            /**< Buy */
    BUSINESS_SELL = 2,           /**< Sell */
    BUSINESS_GIFT = 3,           /**< Bonus share */
    BUSINESS_BONUS = 4,          /**< Cash dividend */
    BUSINESS_CHECKIN = 5,        /**< Deposit cash */
    BUSINESS_CHECKOUT = 6,       /**< Withdraw cash */
    BUSINESS_CHECKIN_STOCK = 7,  /**< Deposit stock assets */
    BUSINESS_CHECKOUT_STOCK = 8, /**< Withdraw stock assets */
    BUSINESS_BORROW_CASH = 9,    /**< Borrow cash */
    BUSINESS_RETURN_CASH = 10,   /**< Repay borrowed cash */
    BUSINESS_BORROW_STOCK = 11,  /**< Borrow stock assets */
    BUSINESS_RETURN_STOCK = 12,  /**< Return borrowed stock assets */
    BUSINESS_SELL_SHORT = 13,    /**< Short sell */
    BUSINESS_BUY_SHORT = 14,     /**< Cover a short position */
    BUSINESS_SUOGU = 15,         /**< Share consolidation / expansion */
    BUSINESS_INVALID = 16        /**< Invalid type */
};

/**
 * Get the business name, used for print output
 * @ingroup TradeManagerClass
 */
string HKU_API getBusinessName(BUSINESS);

/**
 * Get the BUSINESS enum value matching the given string
 */
BUSINESS HKU_API getBusinessEnum(const string&);

/**
 * Trade record
 * @ingroup TradeManagerClass
 */
class HKU_API TradeRecord {
public:
    TradeRecord();
    TradeRecord(const Stock& stock, const Datetime& datetime, BUSINESS business, price_t planPrice,
                price_t realPrice, price_t goalPrice, double number, const CostRecord& cost,
                price_t stoploss, price_t cash, SystemPart from, const string& remark = "");
    TradeRecord(const TradeRecord&) = default;
    TradeRecord& operator=(const TradeRecord&) = default;

    TradeRecord(TradeRecord&&);
    TradeRecord& operator=(TradeRecord&&);

    /** Only used by __str__ in python */
    string toString() const;

    bool isNull() const;

    Stock stock;        ///< Traded security
    Datetime datetime;  ///< Trade date
    BUSINESS business;  ///< Business type
    price_t planPrice;  ///< Planned trade price
    price_t realPrice;  ///< Actual trade price
    price_t goalPrice;  ///< Target price; 0 or Null means no target is set
    double number;      ///< Traded quantity
    CostRecord cost;    ///< Trade cost
    price_t stoploss;   ///< Stop-loss price
    price_t cash;       ///< Cash balance
    SystemPart from;    ///< Auxiliary record of the trading system part that issued the instruction;
                        ///< Null<int>() means invalid
    string remark;      ///< Remark (at most 100 encoded characters)

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        hku::uint64_t date_number = datetime.number();
        ar& bs::make_nvp("datetime", date_number);
        string business_name = getBusinessName(business);
        ar& bs::make_nvp<string>("business", business_name);
        ar& BOOST_SERIALIZATION_NVP(planPrice);
        ar& BOOST_SERIALIZATION_NVP(realPrice);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(cost);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(cash);
        ar& BOOST_SERIALIZATION_NVP(remark);
        string part_name(getSystemPartName(from));
        ar& bs::make_nvp<string>("from", part_name);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        hku::uint64_t date_number;
        ar& bs::make_nvp("datetime", date_number);
        datetime = Datetime(date_number);
        string business_name;
        ar& bs::make_nvp<string>("business", business_name);
        business = getBusinessEnum(business_name);
        ar& BOOST_SERIALIZATION_NVP(planPrice);
        ar& BOOST_SERIALIZATION_NVP(realPrice);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(cost);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(cash);
        ar& BOOST_SERIALIZATION_NVP(remark);
        string part_name;
        ar& bs::make_nvp<string>("from", part_name);
        from = getSystemPartEnum(part_name);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

/**
 * @ingroup TradeManagerClass
 */
typedef vector<TradeRecord> TradeRecordList;

/**
 * Print the TradeRecord information
 * @ingroup TradeManagerClass
 */
HKU_API std::ostream& operator<<(std::ostream&, const TradeRecord&);

bool HKU_API operator==(const TradeRecord& d1, const TradeRecord& d2);

inline bool operator!=(const TradeRecord& d1, const TradeRecord& d2) {
    return !(d1 == d2);
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TradeRecord> : ostream_formatter {};
#endif

#endif /* TRADERECORD_H_ */
