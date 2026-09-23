/*
 * FundsRecord.h
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef FUNDSRECORD_H_
#define FUNDSRECORD_H_

#include "../DataType.h"
#include "../config.h"

namespace hku {

/**
 * Record of the current asset situation, returned by TradeManager::getFunds
 * @ingroup TradeManagerClass
 */
class HKU_API FundsRecord {
public:
    FundsRecord() = default;
    FundsRecord(price_t cash, price_t market_value, price_t short_market_value, price_t base_cash,
                price_t base_asset, price_t borrow_cash, price_t borrow_asset);

    price_t cash{0.0};               /**< Current cash */
    price_t market_value{0.0};       /**< Current long market value */
    price_t short_market_value{0.0}; /**< Current short position market value */
    price_t base_cash{0.0};          /**< Currently invested principal */
    price_t base_asset{0.0};         /**< Value of the currently invested assets */
    price_t borrow_cash{0.0};        /**< Currently borrowed funds, i.e. the liability */
    price_t borrow_asset{0.0};       /**< Value of the currently borrowed security assets */

    // Current total assets = cash + long market value + short quantity × (borrow price - current
    // price)
    //          = cash + market_value + borrow_asset - short_market_value
    // Current liabilities = borrowed funds + value of the borrowed assets
    //         = borrow_cash + borrow_asset
    // Current net assets = total assets - liabilities
    //          = cash + market_value - short_market_value - borrow_cash
    // Currently invested principal value assets = invested principal + invested asset value
    //                = base_cash + base_asset
    // Current profit = current net assets - currently invested principal value assets
    //         = cash + market_value - short_market_value - borrow_cash - base_cash - base_asset

    // Current total assets
    price_t total_assets() const {
        return cash + market_value + borrow_asset - short_market_value;
    }

    // Current net assets
    price_t net_assets() const {
        return cash + market_value - short_market_value - borrow_cash;
    }

    // Total liabilities
    price_t total_borrow() const {
        return borrow_cash + borrow_asset;
    }

    // Currently invested principal value assets
    price_t total_base() const {
        return base_cash + base_asset;
    }

    // Current profit
    price_t profit() const {
        return cash + market_value - short_market_value - borrow_cash - base_cash - base_asset;
    }

    FundsRecord operator+(const FundsRecord& other) const;

    FundsRecord& operator+=(const FundsRecord& other);

    // Serialization support
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(cash);
        ar& BOOST_SERIALIZATION_NVP(market_value);
        ar& BOOST_SERIALIZATION_NVP(short_market_value);
        ar& BOOST_SERIALIZATION_NVP(base_cash);
        ar& BOOST_SERIALIZATION_NVP(base_asset);
        ar& BOOST_SERIALIZATION_NVP(borrow_cash);
        ar& BOOST_SERIALIZATION_NVP(borrow_asset);
    }
#endif
};

typedef vector<FundsRecord> FundsList;
typedef vector<FundsRecord> FundsRecordList;

/**
 * Output the TradeRecord information
 * @ingroup TradeManagerClass
 */
HKU_API std::ostream& operator<<(std::ostream&, const FundsRecord&);

bool HKU_API operator==(const FundsRecord& d1, const FundsRecord& d2);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::FundsRecord> : ostream_formatter {};
#endif

#endif /* FUNDSRECORD_H_ */
