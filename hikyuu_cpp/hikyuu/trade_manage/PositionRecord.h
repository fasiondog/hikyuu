/*
 * PositionRecord.h
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#pragma once
#ifndef POSITIONRECORD_H_
#define POSITIONRECORD_H_

#include "../serialization/PriceSerialization.h"
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
     * @note It is valid for the closed records only; Null (NaN) is returned for the ones not yet
     * closed, so callers can tell "unclosed" apart from a genuine zero profit
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
    price_t margin{0.0};      ///< Margin locked by the holding, 0 means the plain cash accounting
    price_t openBasis{
      0.0};               ///< Opening notional of the still-held lots (price x number x unit); the
                          ///< realized profit and the equity of a margin holding are measured
                          ///< against it, and it is released proportionally as the lots close
    size_t buyCount{0};   ///< Number of buys
    size_t sellCount{0};  ///< Number of sells

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
        std::string gp = priceToSerialString(goalPrice);
        ar& bs::make_nvp("goalPrice", gp);
        ar& BOOST_SERIALIZATION_NVP(totalNumber);
        ar& BOOST_SERIALIZATION_NVP(buyMoney);
        ar& BOOST_SERIALIZATION_NVP(totalCost);
        ar& BOOST_SERIALIZATION_NVP(totalRisk);
        ar& BOOST_SERIALIZATION_NVP(sellMoney);
        ar& BOOST_SERIALIZATION_NVP(buyCount);
        ar& BOOST_SERIALIZATION_NVP(sellCount);
        if (version >= 1) {
            ar& BOOST_SERIALIZATION_NVP(margin);
        }
        if (version >= 2) {
            ar& BOOST_SERIALIZATION_NVP(openBasis);
        }
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
        std::string gp;
        ar& bs::make_nvp("goalPrice", gp);
        goalPrice = priceFromSerialString(gp);
        ar& BOOST_SERIALIZATION_NVP(totalNumber);
        ar& BOOST_SERIALIZATION_NVP(buyMoney);
        ar& BOOST_SERIALIZATION_NVP(totalCost);
        ar& BOOST_SERIALIZATION_NVP(totalRisk);
        ar& BOOST_SERIALIZATION_NVP(sellMoney);
        ar& BOOST_SERIALIZATION_NVP(buyCount);
        ar& BOOST_SERIALIZATION_NVP(sellCount);
        if (version >= 1) {
            ar& BOOST_SERIALIZATION_NVP(margin);
        }
        if (version >= 2) {
            ar& BOOST_SERIALIZATION_NVP(openBasis);
        } else {
            // The old archives carried no remaining cost basis; rebuild the best-effort value the
            // old averaged formula would have produced so the archive stays readable
            price_t notional = buyMoney > 0.0 ? buyMoney : sellMoney;
            openBasis = totalNumber > 0.0 ? notional * (number / totalNumber) : 0.0;
        }
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

#if HKU_SUPPORT_SERIALIZATION
// 0 = the archives without the margin field (plain cash accounting)
// 1 = the margin field, 2 = the remaining opening cost basis
BOOST_CLASS_VERSION(::hku::PositionRecord, 2)
#endif

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::PositionRecord> : ostream_formatter {};
#endif

#endif /* POSITIONRECORD_H_ */
