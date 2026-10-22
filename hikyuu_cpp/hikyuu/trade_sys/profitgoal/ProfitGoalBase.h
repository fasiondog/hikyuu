/*
 * ProfitGoal.h
 *
 *  Created on: 2013-3-7
 *      Author: fasiondog
 */

#pragma once
#ifndef PROFITGOAL_H_
#define PROFITGOAL_H_

#include "../../KData.h"
#include "../../utilities/Parameter.h"
#include "../../trade_manage/TradeManager.h"

namespace hku {

/**
 * Base class of the profit goal strategy
 * @details The profit goal is determined before the trade, it is used by the system to execute a
 * sell when the price reaches the profit goal
 * @ingroup ProfitGoal
 */
class HKU_API ProfitGoalBase {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    ProfitGoalBase();
    explicit ProfitGoalBase(const string& name);
    ProfitGoalBase(const ProfitGoalBase&) = default;
    virtual ~ProfitGoalBase();

    /** Set the account */
    void setTM(const TradeManagerPtr& tm);

    /** Get the account */
    TradeManagerPtr getTM() const;

    /** Set the trading object */
    void setTO(const KData& kdata);

    /** Get the trading object */
    KData getTO() const;

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    /** Receive the actual trade change situation */
    virtual void buyNotify(const TradeRecord&) {}

    /** Receive the actual trade change situation */
    virtual void sellNotify(const TradeRecord&) {}

    /** Reset operation */
    void reset();

    typedef shared_ptr<ProfitGoalBase> ProfitGoalPtr;
    /** Clone interface; the clone does not carry the trading object (to), so setTO must be
     *  called before the clone is used */
    ProfitGoalPtr clone();

    /** Calculate the target price when buying; see the _getGoal interface for the return semantics
     */
    price_t getGoal(const Datetime& datetime, price_t price);

    /** Null<price_t> is returned, meaning no short goal is set */
    price_t getShortGoal(const Datetime& datetime, price_t price);

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual ProfitGoalPtr _clone() = 0;

    /** Subclass calculation interface, it is called by setTO */
    virtual void _calculate() {}

    /**
     * Subclass goal interface, calculate the goal price when buying
     * @param datetime the current time
     * @param price the current price
     * @return Null<price_t> means no goal is set; 0 means a sell is needed. Note: when there
     * is no position yet (e.g. at the buy moment), returning a pre-calculated goal price based
     * on the current price is also a valid semantic, which will be recorded as the goalPrice of
     * the trade record
     */
    virtual price_t _getGoal(const Datetime& datetime, price_t price) = 0;

    /**
     * Subclass short goal interface, calculate the target price when selling short
     * @param datetime the current time
     * @param price the current price
     * @return Null<price_t> means no short goal is set; the short position is closed when the
     * current price is <= the returned target price. To force a close regardless of price (e.g.
     * a hold-days rule), return the current price so that price <= price always holds. Note that
     * 0 has no effect on the short side (price <= 0 never holds)
     */
    virtual price_t _getShortGoal(const Datetime&, price_t) {
        return Null<price_t>();
    }

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

    /** Whether this component or its trade manager is defined in Python; overrides may recurse */
    virtual bool hasPythonPart() const noexcept {
        return m_is_python_object || (m_tm && m_tm->hasPythonPart());
    }

protected:
    bool m_is_python_object{false};
    string m_name;
    KData m_kdata;
    TradeManagerPtr m_tm;

private:
    /** Clamp a mis-implemented goal (negative or non-finite) to Null, i.e. no goal */
    price_t _normalizeGoal(price_t goal);

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
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(ProfitGoalBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public ProfitGoalBase {
 *     PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup ProfitGoal
 */
#define PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                         \
    friend class boost::serialization::access;                   \
    template <class Archive>                                     \
    void serialize(Archive& ar, const unsigned int version) {    \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ProfitGoalBase); \
    }
#else
#define PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define PROFITGOAL_IMP(classname)             \
public:                                       \
    virtual ProfitGoalPtr _clone() override { \
        return std::make_shared<classname>(); \
    }                                         \
    virtual price_t _getGoal(const Datetime&, price_t) override;

/**
 * Client programs should all use this pointer type
 * @ingroup ProfitGoal
 */
typedef shared_ptr<ProfitGoalBase> ProfitGoalPtr;
typedef shared_ptr<ProfitGoalBase> PGPtr;

HKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalBase& pg);
HKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalPtr& pg);

inline void ProfitGoalBase::setTM(const TradeManagerPtr& tm) {
    m_tm = tm;
}

inline TradeManagerPtr ProfitGoalBase::getTM() const {
    return m_tm;
}

inline KData ProfitGoalBase::getTO() const {
    return m_kdata;
}

inline const string& ProfitGoalBase::name() const {
    return m_name;
}

inline void ProfitGoalBase::name(const string& name) {
    m_name = name;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::ProfitGoalBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::ProfitGoalPtr> : ostream_formatter {};
#endif

#endif /* PROFITGOAL_H_ */
