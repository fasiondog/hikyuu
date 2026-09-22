/*
 * TradeCostBase.h
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADECOSTBASE_H_
#define TRADECOSTBASE_H_

#include "../Stock.h"
#include "../utilities/Parameter.h"
#include "CostRecord.h"

namespace hku {

/**
 * Base class of the trade cost algorithm interface
 * @ingroup TradeCost
 */
class HKU_API TradeCostBase {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    TradeCostBase(const string& name);
    virtual ~TradeCostBase();

    typedef shared_ptr<TradeCostBase> TradeCostPtr;
    /** Clone operation   */
    TradeCostPtr clone();

    /** Get the name */
    const string& name() const {
        return m_name;
    }

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const = 0;

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const = 0;

    /**
     * Calculate the cost of borrowing cash
     * @param datetime borrow date
     * @param cash borrowed funds
     */
    virtual CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const {
        return CostRecord();
    }

    /**
     * Calculate the cost of returning the margin financing
     * @param borrow_datetime the date the funds were borrowed
     * @param return_datetime return date
     * @param cash returned amount
     */
    virtual CostRecord getReturnCashCost(const Datetime& borrow_datetime,
                                         const Datetime& return_datetime, price_t cash) const {
        return CostRecord();
    }

    /**
     * Calculate the cost of borrowing securities
     * @param datetime the date of the securities lending
     * @param stock the borrowed object
     * @param price price per share
     * @param num borrowed quantity
     */
    virtual CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const {
        return CostRecord();
    }

    /**
     * Calculate the cost of returning the borrowed securities
     * @param borrow_datetime borrow date
     * @param return_datetime return date
     * @param stock the returned object
     * @param price price per share at returning
     * @param num returned quantity
     */
    virtual CostRecord getReturnStockCost(const Datetime& borrow_datetime,
                                          const Datetime& return_datetime, const Stock& stock,
                                          price_t price, double num) const {
        return CostRecord();
    }

    /** The inheriting subclass must implement the clone interface of its private variables */
    virtual TradeCostPtr _clone() = 0;

protected:
    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    bool m_is_python_object{false};

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(TradeCostBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class DrivedCost: public TradeCostBase {
 *     TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     DrivedCost();
 *     ...
 * };
 * @endcode
 * @ingroup TradeCost
 */
#define TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                        \
    friend class boost::serialization::access;                  \
    template <class Archive>                                    \
    void serialize(Archive& ar, const unsigned int version) {   \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeCostBase); \
    }
#else
#define TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * Trade cost algorithm pointer
 * @ingroup TradeCost
 */
typedef shared_ptr<TradeCostBase> TradeCostPtr;

HKU_API std::ostream& operator<<(std::ostream&, const TradeCostBase&);
HKU_API std::ostream& operator<<(std::ostream&, const TradeCostPtr&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TradeCostBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::TradeCostPtr> : ostream_formatter {};
#endif

#endif /* TRADECOSTBASE_H_ */
