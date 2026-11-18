/*
 * FixedATradeCostBase.h
 *
 *  Created on: 2026-10-9
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_FIXEDATRADECOSTBASE_H_
#define TRADE_MANAGE_IMP_FIXEDATRADECOSTBASE_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Common base of the fixed A-share trade cost models (TC_FixedA / 2015 / 2017 / 2023).
 * @details
 * <pre>
 * It implements the buy/sell cost skeleton shared by all of them:
 *   - commission = max(turnover value * commission, lowest_commission)
 *   - stamp duty charged on sells of A / GEM / STAR / BSE stocks only
 *   - a zero (or non-positive) transaction value yields zero cost
 *   - total = commission + stamp duty (sell only) + transfer fee
 * Only the transfer-fee policy differs by trading era / market scope, and is delegated to the
 * subclasses through _calcTransferFee.
 * </pre>
 * @ingroup TradeCost
 */
class HKU_API FixedATradeCostBase : public TradeCostBase {
public:
    FixedATradeCostBase(const string& name);
    virtual ~FixedATradeCostBase();

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    virtual void _checkParam(const string& name) const override;

protected:
    /**
     * Transfer-fee policy that differs among the eras: either per share or per turnover value,
     * charged on the Shanghai market only or on all markets.
     * @param stock the traded security object
     * @param value turnover value (price * num)
     * @param num traded quantity
     * @return price_t the transfer fee
     */
    virtual price_t _calcTransferFee(const Stock& stock, price_t value, double num) const = 0;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeCostBase);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(FixedATradeCostBase)

/**
 * Serialization macro for a stateless subclass of FixedATradeCostBase
 * @ingroup TradeCost
 */
#define TRADE_COST_FIXEDA_SERIALIZATION                               \
private:                                                              \
    friend class boost::serialization::access;                        \
    template <class Archive>                                          \
    void serialize(Archive& ar, const unsigned int version) {         \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(FixedATradeCostBase); \
    }
#else
#define TRADE_COST_FIXEDA_SERIALIZATION
#endif

}  // namespace hku

#endif /* TRADE_MANAGE_IMP_FIXEDATRADECOSTBASE_H_ */
