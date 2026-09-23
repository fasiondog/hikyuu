/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-30
 *      Author: fasiondog
 */

#pragma once

#include "PositionRecord.h"

namespace hku {
// Extended position information (it is suitable for the one-buy-one-sell case only; for the
// one-buy-multiple-sell case some statistics are inaccurate and are for reference only)
struct HKU_API PositionExtInfo final {
    PositionRecord position;
    price_t maxHighPrice{0.};       // Maximum of the high prices in the period
    price_t minLowPrice{0.0};       // Minimum of the low prices in the period
    price_t maxClosePrice{0.};      // Maximum of the close prices in the period
    price_t minClosePrice{0.0};     // Minimum of the close prices in the period
    price_t currentClosePrice{0.};  // Current close price
    price_t maxPullBack1{0.};       // Maximum drawdown ratio 1 (calculated from the maximum and
                                    // the minimum close prices only) (negative)
    price_t maxPullBack2{0.};       // Maximum drawdown ratio 2 (calculated from the maximum high
                                    // price and the minimum low price in the period) (negative)
    price_t currentProfit{0.};      // Current floating profit and loss (excluding the estimated
                                    // sell cost)

    /** Current drawdown ratio 1 (calculated from the maximum close price and the current close
     *  price only) */
    price_t currentPullBack1() const {
        price_t ret = (maxClosePrice - currentClosePrice) / maxClosePrice;
        return ret > 0. ? 0. : ret;
    }

    /** Current drawdown percentage 2 (calculated from the maximum high price in the period and the
     *  current close price) */
    price_t currentPullBack2() const {
        price_t ret = (maxHighPrice - currentClosePrice) / maxHighPrice;
        return ret > 0. ? 0. : ret;
    }

    /** Maximum floating profit 1 in the period (positive, calculated from the close prices only,
     *  excluding the estimated sell cost; inaccurate when there are multiple trades) */
    price_t maxFloatingProfit1() const {
        price_t ret = maxClosePrice * position.number + position.sellMoney - position.buyMoney;
        return ret < 0. ? 0. : ret;
    }

    /** Maximum floating profit 2 in the period (positive, calculated from the maximum of the high
     *  prices, excluding the estimated sell cost; inaccurate when there are multiple trades) */
    price_t maxFloatingProfit2() const {
        price_t ret = maxHighPrice * position.number + position.sellMoney - position.buyMoney;
        return ret < 0. ? 0. : ret;
    }

    /** Maximum floating loss 1 in the period (negative, calculated from the close prices only,
     *  excluding the estimated sell cost; inaccurate when there are multiple trades) */
    price_t minLossProfit1() const {
        price_t ret = minClosePrice * position.number + position.sellMoney - position.buyMoney;
        return ret > 0. ? 0. : ret;
    }

    /** Maximum floating loss 2 in the period (negative, calculated from the low prices in the
     *  period only, excluding the estimated sell cost; inaccurate when there are multiple trades)
     */
    price_t minLossProfit2() const {
        price_t ret = minLowPrice * position.number + position.sellMoney - position.buyMoney;
        return ret > 0. ? 0. : ret;
    }

    PositionExtInfo() = default;
    PositionExtInfo(const PositionExtInfo&) = default;
    PositionExtInfo& operator=(const PositionExtInfo&) = default;

    PositionExtInfo(PositionExtInfo&& rhs)
    : position(std::move(rhs.position)),
      maxHighPrice(rhs.maxHighPrice),
      minLowPrice(rhs.minLowPrice),
      maxClosePrice(rhs.maxClosePrice),
      minClosePrice(rhs.minClosePrice),
      currentClosePrice(rhs.currentClosePrice),
      maxPullBack1(rhs.maxPullBack1),
      maxPullBack2(rhs.maxPullBack2),
      currentProfit(rhs.currentProfit) {}

    PositionExtInfo& operator=(PositionExtInfo&& rhs) {
        if (this != &rhs) {
            position = std::move(rhs.position);
            maxHighPrice = rhs.maxHighPrice;
            minLowPrice = rhs.minLowPrice;
            maxClosePrice = rhs.maxClosePrice;
            minClosePrice = rhs.minClosePrice;
            currentClosePrice = rhs.currentClosePrice;
            maxPullBack1 = rhs.maxPullBack1;
            maxPullBack2 = rhs.maxPullBack2;
            currentProfit = rhs.currentProfit;
        }
        return *this;
    }

//===================
// Serialization support
//===================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(position);
        ar& BOOST_SERIALIZATION_NVP(maxHighPrice);
        ar& BOOST_SERIALIZATION_NVP(minLowPrice);
        ar& BOOST_SERIALIZATION_NVP(maxClosePrice);
        ar& BOOST_SERIALIZATION_NVP(minClosePrice);
        ar& BOOST_SERIALIZATION_NVP(currentClosePrice);
        ar& BOOST_SERIALIZATION_NVP(maxPullBack1);
        ar& BOOST_SERIALIZATION_NVP(maxPullBack2);
        ar& BOOST_SERIALIZATION_NVP(currentProfit);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(position);
        ar& BOOST_SERIALIZATION_NVP(maxHighPrice);
        ar& BOOST_SERIALIZATION_NVP(minLowPrice);
        ar& BOOST_SERIALIZATION_NVP(maxClosePrice);
        ar& BOOST_SERIALIZATION_NVP(minClosePrice);
        ar& BOOST_SERIALIZATION_NVP(currentClosePrice);
        ar& BOOST_SERIALIZATION_NVP(maxPullBack1);
        ar& BOOST_SERIALIZATION_NVP(maxPullBack2);
        ar& BOOST_SERIALIZATION_NVP(currentProfit);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

}  // namespace hku