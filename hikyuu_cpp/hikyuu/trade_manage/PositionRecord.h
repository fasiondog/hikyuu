/*
 * PositionRecord.h
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#pragma once
#ifndef POSITIONRECORD_H_
#define POSITIONRECORD_H_

#include "../StockManager.h"
#include "../serialization/Stock_serialization.h"

namespace hku {

/**
 * Position record
 * @ingroup TradeManagerClass
 */
class HKU_API PositionRecord {
public:
    PositionRecord() = default;
    PositionRecord(const Stock& stock, const Datetime& takeDatetime, const Datetime& cleanDatetime,
                   double number, price_t stoploss, price_t goalPrice, double totalNumber,
                   price_t buyMoney, price_t totalCost, price_t totalRisk, price_t sellMoney);

    PositionRecord(const PositionRecord& ths) = default;
    PositionRecord& operator=(const PositionRecord& ths) = default;

    PositionRecord(PositionRecord&& rhs);
    PositionRecord& operator=(PositionRecord&& rhs);

    /** Used by __str__ of python only */
    string str() const;

    /**
     * @brief Profit and loss = sell funds - accumulated total trade cost - buy funds
     * @note It is valid for the closed records only, 0.0 is returned for the ones not yet closed
     */
    price_t totalProfit() const;

    Stock stock;              ///< Trading object
    Datetime takeDatetime;    ///< Date of the first position opening
    Datetime cleanDatetime;   ///< Closing date, it is Null<Datetime>() in the current position
                              ///< records
    double number{0.0};       ///< Current held quantity
    price_t stoploss{0.0};    ///< Current stop-loss price
    price_t goalPrice{0.0};   ///< Current target price
    double totalNumber{0.0};  ///< Accumulated held quantity
    price_t buyMoney{0.0};    ///< Accumulated buy funds
    price_t totalCost{0.0};   ///< Accumulated total trade cost
    price_t totalRisk{0.0};   ///< Accumulated trade risk = sum of (buy price - stop-loss) * buy
                              ///< quantity, excluding the trade cost
    price_t sellMoney{0.0};   ///< Accumulated sell funds
    size_t buyCount{0};       ///< Number of buys
    size_t sellCount{0};      ///< Number of sells

//===================
// Serialization support
//===================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        uint64_t take = takeDatetime.number();
        uint64_t clean = cleanDatetime.number();
        ar& bs::make_nvp("takeDatetime", take);
        ar& bs::make_nvp("cleanDatetime", clean);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(totalNumber);
        ar& BOOST_SERIALIZATION_NVP(buyMoney);
        ar& BOOST_SERIALIZATION_NVP(totalCost);
        ar& BOOST_SERIALIZATION_NVP(totalRisk);
        ar& BOOST_SERIALIZATION_NVP(sellMoney);
        ar& BOOST_SERIALIZATION_NVP(buyCount);
        ar& BOOST_SERIALIZATION_NVP(sellCount);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        uint64_t take, clean;
        ar& bs::make_nvp("takeDatetime", take);
        ar& bs::make_nvp("cleanDatetime", clean);
        takeDatetime = Datetime(take);
        cleanDatetime = Datetime(clean);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(totalNumber);
        ar& BOOST_SERIALIZATION_NVP(buyMoney);
        ar& BOOST_SERIALIZATION_NVP(totalCost);
        ar& BOOST_SERIALIZATION_NVP(totalRisk);
        ar& BOOST_SERIALIZATION_NVP(sellMoney);
        ar& BOOST_SERIALIZATION_NVP(buyCount);
        ar& BOOST_SERIALIZATION_NVP(sellCount);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/** @ingroup TradeManagerClass */
typedef vector<PositionRecord> PositionRecordList;

/**
 * Output the position record information
 * @ingroup TradeManagerClass
 */
HKU_API std::ostream& operator<<(std::ostream&, const PositionRecord&);

bool HKU_API operator==(const PositionRecord& d1, const PositionRecord& d2);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::PositionRecord> : ostream_formatter {};
#endif

#endif /* POSITIONRECORD_H_ */
