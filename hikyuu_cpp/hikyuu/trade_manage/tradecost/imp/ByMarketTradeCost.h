/*
 * ByMarketTradeCost.h
 *
 *  Created on: 2026-11-12
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_BYMARKETTRADECOST_H_
#define TRADE_MANAGE_IMP_BYMARKETTRADECOST_H_

#include <map>
#include <mutex>
#include <set>

#include "../TradeCostBase.h"

namespace hku {

/**
 * Routes the cost calculation to another cost model by the market or the security type
 * @details It answers no fee itself: every query is forwarded to the model registered for the
 * security, so the fee schedule of each market stays in its own existing model (TC_FixedA, TC_HK,
 * TC_Fixed, ...). A security is matched by its market code first (e.g. "HK"), then by its security
 * type (e.g. "TYPE:12"), and falls back to the default model when neither is registered; an
 * unmatched market is reported once and then billed by the default model.
 * The account-level hooks that carry no security (the borrowed cash cost and rate) are answered
 * by the default model. Configure the routing before the run: the table is not synchronized with
 * the queries, and only the one-off unmatched warning takes a lock.
 * @see TC_ByMarket
 * @ingroup TradeCost
 */
class HKU_API ByMarketTradeCost : public TradeCostBase {
public:
    /** Every security is billed by a zero-cost model until a model is registered */
    ByMarketTradeCost();

    /**
     * @param default_cost the model answering an unregistered market, a zero-cost model when null
     */
    explicit ByMarketTradeCost(const TradeCostPtr& default_cost);

    virtual ~ByMarketTradeCost();

    /**
     * Register the cost model of a key, an existing key is replaced
     * @param key the market code (e.g. "HK", case insensitive) or "type:<n>" (e.g. "type:12")
     * @param cost the cost model answering that key, ignored when null
     * @throws hku::exception an empty key, or a "type:" key without the type number
     * @note Configure before the run, the routing table is not synchronized with the queries
     */
    void add(const string& key, const TradeCostPtr& cost);

    /** The model answering an unregistered market, a null pointer restores the zero-cost model */
    void setDefault(const TradeCostPtr& cost);

    /** The model answering an unregistered market */
    const TradeCostPtr& getDefault() const {
        return m_default;
    }

    /** The model registered for the key (not the default), a null pointer when absent */
    TradeCostPtr get(const string& key) const;

    /** The registered keys, normalized to upper case, in ascending order */
    vector<string> keys() const;

    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    /** Answered by the default model: the borrowed cash is not attributed to a market */
    virtual CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const override;

    virtual CostRecord getReturnCashCost(const Datetime& borrow_datetime,
                                         const Datetime& return_datetime,
                                         price_t cash) const override;

    virtual CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const override;

    virtual CostRecord getReturnStockCost(const Datetime& borrow_datetime,
                                          const Datetime& return_datetime, const Stock& stock,
                                          price_t price, double num) const override;

    /** Answered by the default model: the borrowed cash is not attributed to a market */
    virtual double getBorrowCashRate(const Datetime& datetime) const override;

    virtual double getBorrowStockRate(const Datetime& datetime, const Stock& stock) const override;

    /** A Python defined model anywhere in the routing must keep the GIL too */
    virtual bool hasPythonPart() const noexcept override;

    virtual TradeCostPtr _clone() override;

private:
    /** The model of the security: market code, then "TYPE:<n>", then the default */
    const TradeCostPtr& _route(const Stock& stock) const;

private:
    std::map<string, TradeCostPtr> m_costs;  // key (upper case) -> cost model
    TradeCostPtr m_default;                  // never null
    mutable std::mutex m_warn_mutex;
    mutable std::set<string> m_warned;  // markets already reported as unmatched, not serialized

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeCostBase);
        ar& BOOST_SERIALIZATION_NVP(m_costs);
        ar& BOOST_SERIALIZATION_NVP(m_default);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_BYMARKETTRADECOST_H_ */
