/*
 * MarginInterestTradeCost.h
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_MARGIN_INTEREST_TRADE_COST_H_
#define TRADE_MANAGE_IMP_MARGIN_INTEREST_TRADE_COST_H_

#include "../../RateTable.h"
#include "../TradeCostBase.h"

namespace hku {

/**
 * Wrapper of any cost model, adding the interest of the margin financing and the securities lending
 * @details The wrapped model answers trading fees; this class bills only debts: the account queries
 * both rates in daily settlement, prorates them over the calendar days since the last billing and
 * adds the result to the debt rather than paying it from cash. Both rates are 0.0 by default, so
 * wrapping alone bills nothing.
 * @see TC_MarginInterest
 * @ingroup TradeCost
 */
class HKU_API MarginInterestTradeCost : public TradeCostBase {
public:
    /** Wrap a zero-cost model, i.e. bill nothing but the interest */
    MarginInterestTradeCost();

    /**
     * Wrap another cost model
     * @param trade_cost the model answering the trading fees; null falls back to a zero-cost model
     */
    explicit MarginInterestTradeCost(const TradeCostPtr& trade_cost);

    virtual ~MarginInterestTradeCost();

    /** The wrapped model answering the trading fees */
    const TradeCostPtr& getInnerCost() const {
        return m_inner;
    }

    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    virtual CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const override;

    virtual CostRecord getReturnCashCost(const Datetime& borrow_datetime,
                                         const Datetime& return_datetime,
                                         price_t cash) const override;

    virtual CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const override;

    virtual CostRecord getReturnStockCost(const Datetime& borrow_datetime,
                                          const Datetime& return_datetime, const Stock& stock,
                                          price_t price, double num) const override;

    /** The annualized rate of the borrowed cash, the borrow_cash_rate param */
    virtual double getBorrowCashRate(const Datetime& datetime) const override;

    /** The annualized rate of the borrowed security, matched by the borrow_stock_rates table */
    virtual double getBorrowStockRate(const Datetime& datetime, const Stock& stock) const override;

    /** Also keep the GIL when the inner model is Python-defined */
    virtual bool hasPythonPart() const noexcept override {
        return m_is_python_object || (m_inner && m_inner->hasPythonPart());
    }

    virtual TradeCostPtr _clone() override;

protected:
    virtual void _checkParam(const string& name) const override;

private:
    TradeCostPtr m_inner;  // The model answering the trading fees

    // Derived from borrow_stock_rates, so not serialized; _checkParam rebuilds it on every check
    // (set_param, clone, deserialization)
    mutable RateTable m_stock_rates;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeCostBase);
        ar& BOOST_SERIALIZATION_NVP(m_inner);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_MARGIN_INTEREST_TRADE_COST_H_ */
