/*
 * MarginBase.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_MARGINGBASE_H_
#define TRADE_MANAGE_MARGINGBASE_H_

#include "../../Stock.h"
#include "../../utilities/Parameter.h"

namespace hku {

/** Side of an opened position */
enum class MarginSide { LONG, SHORT };

/**
 * Base class of the margin component, i.e. how much funds an opening consumes
 * @details The component only answers questions about the margin quota and thresholds; it never
 *        touches the account state, the market data or the trade cost:
 *        - funds out of the account on opening: the margin, answered here;
 *        - fees and interest: TradeCostBase;
 *        - bookkeeping and order validation: TradeManagerBase.
 *        A subclass must stay a pure function of its parameters: no StockManager lookup, no K-line
 *        access, no account mutation, since it is queried on every order of every bar.
 * @ingroup Margin
 */
class HKU_API MarginBase {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    MarginBase(const string& name);
    virtual ~MarginBase();

    typedef shared_ptr<MarginBase> MarginPtr;

    /** Clone operation */
    MarginPtr clone();

    /** Get the name */
    const string& name() const {
        return m_name;
    }

    /**
     * Whether the security is covered by the margin accounting
     * @details false means it is a plain cash position: it takes no part in the occupied margin,
     *          so it does not reduce the available funds of a mixed account
     */
    virtual bool isMarginTrading(const Stock& stock) const {
        return true;
    }

    /**
     * Whether opening a short of the security needs borrowing the asset
     * @details The default keeps the A-share securities lending semantics for the built-in
     *          categories, while a cryptocurrency and any custom type (>= 12, i.e. futures, forex,
     *          US or HK stocks registered for a new market) open a short without borrowing. A
     *          subclass may override it when the market has another rule.
     */
    virtual bool needBorrowAsset(const Stock& stock) const;

    /**
     * Margin occupied by an opening
     * @param datetime trade date
     * @param stock the traded security
     * @param price opening price
     * @param num opening quantity, it may be 1.0 to get the margin of a single unit
     * @param side long or short
     * @return occupied funds, 0 when the inputs are not positive or finite
     */
    virtual price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num, MarginSide side) const = 0;

    /** Maintain margin, i.e. the additional-margin and forced-liquidation threshold */
    virtual price_t getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num, MarginSide side) const;

    /**
     * Ratio view of the margin, kept compatible with the former TradeManager::getMarginRate
     * @return a value in (0, 1]; 1.0 means no leverage, and the per-lot mode always returns 1.0
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock,
                                 MarginSide side) const;

    /**
     * Maximum openable quantity under the available funds
     * @param available available funds
     * @return aligned down to stock.minTradeNumber() and capped by stock.maxTradeNumber(); 0 when
     *         nothing can be opened
     */
    virtual double getMaxOpenNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                    price_t available, MarginSide side) const;

    /** The inheriting subclass must implement the clone interface of its private variables */
    virtual MarginPtr _clone() = 0;

    /** Whether this margin component is defined in Python */
    virtual bool hasPythonPart() const noexcept {
        return m_is_python_object;
    }

protected:
    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

    /** Nominal value of a position = price * num * unit; 0 for inputs that are not positive */
    static price_t nominal(const Stock& stock, price_t price, double num);

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
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        // The node name must stay m_params to match the archives written before the margin model
        // existed. setParameter instead of a raw assignment: it re-runs the param check, which is
        // the only place a model may rebuild the state it derives from its params, so the hot path
        // stays a pure table lookup and a shared model can be queried from several threads
        ar& boost::serialization::make_nvp("m_params", const_cast<Parameter&>(m_params));
        setParameter(m_params);
    }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        boost::serialization::split_member(ar, *this, version);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MarginBase)
#endif

/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @ingroup Margin
 */
#if HKU_SUPPORT_SERIALIZATION
#define MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION                \
private:                                                      \
    friend class boost::serialization::access;                \
    template <class Archive>                                  \
    void serialize(Archive& ar, const unsigned int version) { \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MarginBase);  \
    }
#else
#define MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * Margin component pointer
 * @ingroup Margin
 */
typedef shared_ptr<MarginBase> MarginPtr;

HKU_API std::ostream& operator<<(std::ostream&, const MarginBase&);
HKU_API std::ostream& operator<<(std::ostream&, const MarginPtr&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MarginBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::MarginPtr> : ostream_formatter {};
#endif

#endif /* TRADE_MANAGE_MARGINGBASE_H_ */
