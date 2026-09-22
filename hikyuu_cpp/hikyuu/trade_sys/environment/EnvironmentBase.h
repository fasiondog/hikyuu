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
 * Base class of the environment judgment strategy
 * @note The external environment should have nothing to do with the concrete trading object
 * @ingroup Environment
 */
class HKU_API EnvironmentBase : public enable_shared_from_this<EnvironmentBase> {
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

    /** Reset */
    void reset();

    /** Set the query condition */
    void setQuery(const KQuery& query);

    /** Get the query condition */
    const KQuery& getQuery() const {
        return m_query;
    }

    typedef shared_ptr<EnvironmentBase> EnvironmentPtr;
    /**
     * Clone operation
     * @note Unlike the other system strategy components, the Environment is not bound to a specific
     *       trading object and can be shared, so essentially the clone operation is not needed
     * here; it exists only for the consistency and the possibly existing special scenarios.
     */
    EnvironmentPtr clone();

    /**
     * Add a valid time, it is called in _calculate
     * @param datetime the valid date of the system
     * @param value 1.0 by default; greater than 0 means valid and less than or equal to 0 means
     *              invalid
     */
    void _addValid(const Datetime& datetime, price_t value = 1.0);

    /**
     * Judge whether the external environment of the given date is valid
     * @param datetime the given date
     * @return true valid | false invalid
     */
    bool isValid(const Datetime& datetime) const;

    price_t getValue(const Datetime& datetime) const;

    /**
     * Get the actual value in the form of an indicator, it is as long as the trading object; <=0
     * means invalid and >0 means the system is valid
     * @note A time series indicator with the dates
     */
    Indicator getValues() const;

    /** Subclass calculation interface */
    virtual void _calculate() = 0;

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual EnvironmentPtr _clone() = 0;

    bool isPythonObject() const noexcept {
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
