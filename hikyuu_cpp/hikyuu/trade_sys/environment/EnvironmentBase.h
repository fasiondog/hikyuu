/*
 * Environment.h
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#pragma once
#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <set>
#include <shared_mutex>
#include "../../KQuery.h"
#include "../../utilities/Parameter.h"
#include "hikyuu/indicator/Indicator.h"

namespace hku {

/**
 * Base class of the market environment strategy
 * @note The external environment should have nothing to do with the concrete trading object
 * @note The environment is designed to be shared across systems (System shared_ev defaults to
 *       true): isValid takes a shared lock on every call while setQuery/reset take the exclusive
 *       lock, so all systems sharing one environment must use the same query, otherwise every
 *       differing setQuery reruns the full calculation under the exclusive lock
 * @ingroup Environment
 */
class HKU_API EnvironmentBase {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    EnvironmentBase();
    explicit EnvironmentBase(const string& name);
    virtual ~EnvironmentBase();

    // Used for the python clone, but it is not thread safe because of the mutex
    EnvironmentBase(const EnvironmentBase&);

    /** Get the name */
    const string& name() const {
        return m_name;
    }

    /** Set the name */
    void name(const string& name) {
        m_name = name;
    }

    /**
     * Reset
     * @note For a Python subclass the _reset callback acquires the GIL while the mutex is held, so
     *       reset must not be invoked concurrently with the other calls on the same object
     */
    void reset();

    /**
     * Set the query condition; the calculation is re-run only when the query differs from the
     * current one
     * @note For a Python subclass the _calculate callback acquires the GIL while the mutex is
     *       held, so setQuery must not be invoked concurrently with the other calls on the same
     *       object
     */
    void setQuery(const KQuery& query);

    /** Get the query condition */
    const KQuery& getQuery() const {
        return m_query;
    }

    typedef shared_ptr<EnvironmentBase> EnvironmentPtr;
    /**
     * Clone operation
     * @note Unlike the other trading-system parts, the Environment is not bound to a specific
     *       instrument and can be shared, so essentially the clone operation is not needed
     * here; it exists only for the consistency and the possibly existing special scenarios.
     * @note The subclass _clone callback is invoked without holding the mutex, so that it may
     *       reach the GIL (or re-enter this object) without risking a lock-order deadlock.
     */
    EnvironmentPtr clone();

    /**
     * Add a valid time, it is called in _calculate
     * @param datetime the valid date of the system
     * @param value 1.0 by default; greater than 0 means valid and less than or equal to 0 means
     *              invalid
     * @note A non-finite value (NaN/inf) is discarded, and adding the same date twice accumulates
     *       the value instead of replacing it
     * @note It takes no lock itself, so it must only be called from _calculate (or another hook
     *       invoked by setQuery / reset while the mutex is held); a call from anywhere else would
     *       race with the readers
     */
    void _addValid(const Datetime& datetime, price_t value = 1.0);

    /**
     * Judge whether the external environment of the given date is valid
     * @param datetime the given date
     * @return true valid | false invalid
     * @note A date that was never added is invalid; for a registered date the rule is delegated
     *       to the _isValid hook, whose default is "the stored value is finite and > 0"
     * @note For a Python subclass that overrides _is_valid, the callback acquires the GIL while
     *       the shared lock is held, so isValid must not run concurrently with setQuery/reset
     *       on the same object
     */
    bool isValid(const Datetime& datetime) const;

    /**
     * Get the stored value of the given date
     * @param datetime the given date
     * @return the stored value, or 0.0 for a date that was never added
     * @note 0.0 is returned both for an invalid date and for an absent one; use isValid to tell
     *       the validity
     */
    price_t getValue(const Datetime& datetime) const;

    /**
     * Get the values as a time-series indicator: one record per date registered through _addValid,
     * ordered by date; only a finite value > 0 means valid, anything else is invalid
     * @note The date set and the meaning of the value depend on the operator that produced this
     *       object, see the operator notes above
     */
    Indicator getValues() const;

    /**
     * Get the raw value storage; for each date d in dateIndexRef(), the element
     * valuesRef()[dateIndexRef()[d]] is the value of d. The vector itself is NOT guaranteed
     * to be stored in date-ascending order (only dateIndexRef() iterates ascending).
     * @note Read-only reference. Use this inside combination operators to avoid the
     *       Indicator/PRICELIST copy produced by getValues(). Valid only while the query is
     *       stable (no concurrent setQuery on this object).
     */
    const vector<price_t>& valuesRef() const {
        return m_values;
    }

    /**
     * Get the date->index ordered map; iterating it yields dates in ascending order.
     * @note Read-only reference, for internal combination without copying.
     */
    const map<Datetime, size_t>& dateIndexRef() const {
        return m_date_index;
    }

    /**
     * Combination operators intentionally use different strategies over these refs, driven by
     * their result semantics:
     *  - And / Multi / Div (intersection): two-pointer merge over the two ordered date_index
     *    maps, so there is no base side to pre-fill. And keeps only the dates valid (> 0) on both
     *    sides, with the value fixed to 1.0. Multi / Div keep only the dates where both sides
     *    agree: both valid (> 0) gives the product / ratio, both invalid (< 0) gives its negation
     *    (kept negative so that the day stays invalid) - a disagreement (mixed sign) or a zero
     *    side carries no combined strength and is left out.
     *  - Add / Sub (ev1 base + ev2 align): every date (valid or not) of ev1 and ev2 belongs to
     *    the result, so ev1 is laid down as the base and ev2 is aligned against it; ev1-only
     *    dates need no work and ev2-only dates go through _addValid. The value is the sum (Add) /
     *    the difference (Sub), an ev2-only date contributing -ev2 to Sub.
     *  - Or (union of valid): a date is kept iff it is valid on at least one side, with the value
     *    fixed to 1.0, so both maps are walked and the date is pushed only when a side is valid.
     * Do NOT "unify" these: a laydown for Or would add ev1's invalid dates as 0.0 to the output
     * date set; an intersection for Add/Sub would drop ev1-only/ev2-only dates; a laydown for
     * And/Multi/Div would wrongly keep ev1-only dates. Each file's _calculate spells out its own
     * branch.
     */

    /**
     * The subclass validity-rule hook, called by isValid for every date registered through
     * _addValid
     * @param datetime the queried date
     * @param value the stored value of the date
     * @return true valid | false invalid; the default rule is "the value is finite and > 0"
     * @note The System judges the market validity solely through isValid (one call per bar), so
     *       overriding this hook directly changes the trading behavior
     * @note Override it to apply a custom rule based on the concrete ev value; the combination
     *       operators keep using the built-in "> 0" rule on their sides
     */
    virtual bool _isValid(const Datetime& datetime, price_t value) const;

    /** Subclass calculation interface */
    virtual void _calculate() = 0;

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual EnvironmentPtr _clone() = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

    /** Whether this component itself is defined in Python; subclasses with Python parts recurse */
    virtual bool hasPythonPart() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    KQuery m_query;
    map<Datetime, size_t> m_date_index;
    vector<price_t> m_values;
    mutable std::shared_mutex m_mutex;

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
        // ev may be shared by multiple systems; m_query is kept and may be used for the
        // troubleshooting
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_date_index);
        ar& BOOST_SERIALIZATION_NVP(m_values);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_date_index);
        ar& BOOST_SERIALIZATION_NVP(m_values);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(EnvironmentBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public EnvironmentBase {
 *     ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Environment
 */
#define ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                          \
    friend class boost::serialization::access;                    \
    template <class Archive>                                      \
    void serialize(Archive& ar, const unsigned int version) {     \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase); \
    }
#else
#define ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * Client programs should all use this pointer type
 * @ingroup Environment
 */
typedef shared_ptr<EnvironmentBase> EnvironmentPtr;
typedef shared_ptr<EnvironmentBase> EVPtr;

#define ENVIRONMENT_IMP(classname)             \
public:                                        \
    virtual EnvironmentPtr _clone() override { \
        return std::make_shared<classname>();  \
    }                                          \
    virtual void _calculate() override;

/**
 * Output the Environment information, e.g. Environment(name, params[...])
 * @ingroup Environment
 */
HKU_API std::ostream& operator<<(std::ostream& os, const EnvironmentPtr&);
HKU_API std::ostream& operator<<(std::ostream& os, const EnvironmentBase&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::EnvironmentBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::EnvironmentPtr> : ostream_formatter {};
#endif

#endif /* ENVIRONMENT_H_ */
