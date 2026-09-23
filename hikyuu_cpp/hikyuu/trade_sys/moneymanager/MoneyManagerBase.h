/*
 * MoneyManagerBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef MONEYMANAGERBASE_H_
#define MONEYMANAGERBASE_H_

#include "../../utilities/Parameter.h"
#include "../system/SystemPart.h"
#include "../../trade_manage/TradeManager.h"

namespace hku {

/**
 * Base class of the money management (**single system/single security** form)
 * @details Responsibility: given the market data, price, risk and cash of a single instrument (its
 *          own account), it decides the buy/sell quantity. The portfolio-level fund allocation
 *          (L1/L2/L3) has been migrated to AllocateFundsBase (AF); the two no longer share the class
 *          hierarchy, the parameter family or the allocation mode.
 * @ingroup MoneyManager
 */
class HKU_API MoneyManagerBase : public enable_shared_from_this<MoneyManagerBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    MoneyManagerBase();
    explicit MoneyManagerBase(const string& name);
    MoneyManagerBase(const MoneyManagerBase&) = default;
    virtual ~MoneyManagerBase();

    /** Get the name */
    const string& name() const {
        return m_name;
    }

    /** Set the name */
    void name(const string& name) {
        m_name = name;
    }

    /** Reset */
    void reset();

    /**
     * Set the trade account
     * @param tm the given trade account
     */
    void setTM(const TradeManagerPtr& tm) {
        m_tm = tm;
    }

    /**
     * Get the trade account
     * @return
     */
    TradeManagerPtr getTM() const {
        return m_tm;
    }

    /** Set the query condition */
    void setQuery(const KQuery& query) {
        m_query = query;
    }

    /** Get the K-line type of the trade */
    const KQuery& getQuery() const {
        return m_query;
    }

    typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
    /** Clone operation */
    MoneyManagerPtr clone();

    /** Receive the actual trade change situation */
    void buyNotify(const TradeRecord& tr);

    /** Interface for the subclass to receive the actual trade change situation; it generally needs
     *  to be overloaded only when there are multiple position increases and decreases */
    virtual void _buyNotify(const TradeRecord&) {}

    /** Receive the actual trade change situation */
    void sellNotify(const TradeRecord& tr);

    /** Interface for the subclass to receive the actual trade change situation; it generally needs
     *  to be overloaded only when there are multiple position increases and decreases */
    virtual void _sellNotify(const TradeRecord&) {}

    /**
     * Get the quantity of the given trading object that can be sold
     * @param datetime trade date
     * @param stock the trading object
     * @param price trade price
     * @param risk the risk taken by the new trade; if it is 0, it means the whole loss, i.e. the
     *             market value drops to 0 yuan
     * @param from signal source
     * @note The default implementation returns MAX_DOUBLE, i.e. selling everything; this interface
     * is needed only for the multiple position reductions
     */
    double getSellNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                         SystemPart from);

    /**
     * Get the quantity of the given trading object that can be short sold
     * @param datetime trade date
     * @param stock the trading object
     * @param price trade price
     * @param from signal source
     * @param risk the trade risk taken; Null<price_t> means there is no loss upper limit
     */
    double getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                              price_t risk, SystemPart from);

    /**
     * Get the buy quantity to cover the short position of the given trading object
     * @param datetime trade date
     * @param stock the trading object
     * @param price trade price
     * @param from signal source
     * @param risk the trade risk taken; Null<price_t> means there is no loss upper limit
     */
    double getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                             price_t risk, SystemPart from);

    /**
     * Get the quantity of the given trading object that can be bought
     * @param datetime trade date
     * @param stock the trading object
     * @param price trade price
     * @param from signal source
     * @param risk the risk taken by the trade; if it is 0, it means the whole loss, i.e. the market
     *             value drops to 0 yuan
     */
    double getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                        SystemPart from);

    /** Current number of the buy trades; it counts the consecutive buys and is reset to 0 once a
     *  sell is received */
    size_t currentBuyCount(const Stock&) const;

    /** Current number of the sell trades; it counts the consecutive sells and is reset to 0 once a
     *  buy is received */
    size_t currentSellCount(const Stock&) const;

    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                 price_t risk, SystemPart from) = 0;

    virtual double _getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                  price_t risk, SystemPart from);

    virtual double _getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from);

    virtual double _getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                      price_t risk, SystemPart from);

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Interface for the subclass to clone its private variables */
    virtual MoneyManagerPtr _clone() = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    KQuery m_query;
    TradeManagerPtr m_tm;
    unordered_map<Stock, std::pair<size_t, size_t>> m_buy_sell_counts;
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
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        // v5: m_mode has been migrated to AllocateFundsBase (portfolio-level allocation), it is no
        // longer saved.
        // m_query and m_tm are set temporarily when the system runs, they do not need to be
        // serialized
        // ar & BOOST_SERIALIZATION_NVP(m_query);
        // ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        if (version < 1) {
            // v5 compatibility: m_mode was at this position in the old archives (the portfolio-level allocation mode), it is discarded after being read.
            string legacy_mode = "A";
            ar& boost::serialization::make_nvp("m_mode", legacy_mode);
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MoneyManagerBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public MoneyManagerBase {
 *     MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MoneyManager
 */
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                           \
    friend class boost::serialization::access;                     \
    template <class Archive>                                       \
    void serialize(Archive& ar, const unsigned int version) {      \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase); \
    }
#else
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * Client programs should all use this pointer type
 * @ingroup MoneyManager
 */
typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
typedef shared_ptr<MoneyManagerBase> MMPtr;

#define MONEY_MANAGER_IMP(classname)                                                          \
public:                                                                                       \
    virtual MoneyManagerPtr _clone() override {                                               \
        return std::make_shared<classname>();                                                 \
    }                                                                                         \
    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price, \
                                 price_t risk, SystemPart from) override;

HKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerBase&);
HKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerPtr&);

} /* namespace hku */

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_VERSION(::hku::MoneyManagerBase, 1)
#endif

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MoneyManagerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::MoneyManagerPtr> : ostream_formatter {};
#endif

#endif /* MONEYMANAGERBASE_H_ */
