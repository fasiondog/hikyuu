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
 * @details The trading fees are answered by the wrapped model exactly as they were; this class
 * bills only the debts: the account queries both rates in its daily settlement and prorates them
 * over the calendar days gone by since the book was last billed, adding the result to the debt
 * itself instead of paying it from the cash.
 * Both rates are 0.0 by default, so wrapping a model bills nothing until a rate is configured.
 * @see TC_MarginInterest
 * @ingroup TradeCost
 */
class HKU_API MarginInterestTradeCost : public TradeCostBase {
public:
    /** Wrap a zero-cost model, i.e. bill nothing but the interest */
    MarginInterestTradeCost();

    /**
     * Wrap another cost model
     * @param trade_cost the model answering the trading fees, the wrapper falls back to a
     *        zero-cost model when it is null
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

    /** A Python defined inner model must keep the GIL too, so the flag propagates through it */
    virtual bool hasPythonPart() const noexcept override {
        return m_is_python_object || (m_inner && m_inner->hasPythonPart());
    }

    virtual TradeCostPtr _clone() override;

protected:
    virtual void _checkParam(const string& name) const override;

private:
    TradeCostPtr m_inner;  // The model answering the trading fees

    // The table is derived from the borrow_stock_rates param, hence it is not serialized; it is
    // rebuilt by _checkParam at every param check: set_param, clone and deserialization
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
