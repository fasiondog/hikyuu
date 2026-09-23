/*
 * ConditionBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef CONDITIONBASE_H_
#define CONDITIONBASE_H_

#include "../../utilities/Parameter.h"
#include "../../KData.h"
#include "../../trade_manage/TradeManager.h"
#include "../signal/SignalBase.h"

namespace hku {

/**
 * Base class of the system valid condition
 * @note The system validity may or may not be related to the object to be traded, so the
 *       setTradeObj interface is kept
 * @ingroup Condition
 */
class HKU_API ConditionBase : public enable_shared_from_this<ConditionBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    ConditionBase();
    ConditionBase(const ConditionBase&) = default;
    explicit ConditionBase(const string& name);
    virtual ~ConditionBase();

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    size_t size() const;

    price_t at(size_t pos) const;

    price_t const* data() const;

    /** Reset operation */
    void reset();

    /** Set the trading object */
    void setTO(const KData& kdata);

    /** Get the trading object */
    KData getTO() const;

    /** Set the trade management instance */
    void setTM(const TradeManagerPtr& tm);

    /** Get the trade management instance */
    TradeManagerPtr getTM() const;

    /** Set the system signal generator */
    void setSG(const SGPtr& sg);

    /** Get the system signal generator */
    SGPtr getSG() const;

    /** Get the valid date list of the system; note: it is not as long as the trading object */
    DatetimeList getDatetimeList() const;

    /**
     * Get the actual value in the form of an indicator, it is as long as the trading object; <=0
     * means invalid and >0 means the system is valid
     * @note A time series indicator with the dates
     */
    Indicator getValues() const;

    /**
     * Add a valid time, it is called in _calculate
     * @param datetime the valid date of the system
     * @param value the value
     */
    void _addValid(const Datetime& datetime, price_t value = 1.0);

    typedef shared_ptr<ConditionBase> ConditionPtr;
    /** Clone operation */
    ConditionPtr clone();

    /**
     * Whether the system is valid at the given time
     * @param datetime the given time
     * @return true valid | false invalid
     */
    bool isValid(const Datetime& datetime);

    /** Subclass calculation interface */
    virtual void _calculate() = 0;

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual ConditionPtr _clone() = 0;

public:
    typedef vector<price_t>::const_iterator const_iterator;
    const_iterator cbegin() const {
        return m_values.cbegin();
    }

    const_iterator cend() const {
        return m_values.cend();
    }

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    KData m_kdata;
    TMPtr m_tm;
    SGPtr m_sg;
    map<Datetime, size_t> m_date_index;
    vector<price_t> m_values;
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
        ar& BOOST_SERIALIZATION_NVP(m_date_index);
        ar& BOOST_SERIALIZATION_NVP(m_values);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        // m_kdata/m_tm/m_sg are set temporarily when the system runs, they do not need to be
        // serialized
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_date_index);
        ar& BOOST_SERIALIZATION_NVP(m_values);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        // m_kdata/m_tm/m_sg are set temporarily when the system runs, they do not need to be
        // serialized
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(ConditionBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public ConditionBase {
 *     CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Condition
 */
#define CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                        \
    friend class boost::serialization::access;                  \
    template <class Archive>                                    \
    void serialize(Archive& ar, const unsigned int version) {   \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); \
    }
#else
#define CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * Client programs should all use this pointer type
 * @ingroup Condition
 */
typedef shared_ptr<ConditionBase> ConditionPtr;
typedef shared_ptr<ConditionBase> CNPtr;

#define CONDITION_IMP(classname)              \
public:                                       \
    virtual ConditionPtr _clone() override {  \
        return std::make_shared<classname>(); \
    }                                         \
    virtual void _calculate() override;

HKU_API std::ostream& operator<<(std::ostream&, const ConditionPtr&);
HKU_API std::ostream& operator<<(std::ostream&, const ConditionBase&);

inline const string& ConditionBase::name() const {
    return m_name;
}

inline void ConditionBase::name(const string& name) {
    m_name = name;
}

inline size_t ConditionBase::size() const {
    return m_values.size();
}

inline price_t const* ConditionBase::data() const {
    return m_values.data();
}

inline price_t ConditionBase::at(size_t pos) const {
    return m_values.at(pos);
}

inline KData ConditionBase::getTO() const {
    return m_kdata;
}

inline void ConditionBase::setTM(const TradeManagerPtr& tm) {
    m_tm = tm;
}

inline SGPtr ConditionBase::getSG() const {
    return m_sg;
}

inline void ConditionBase::setSG(const SGPtr& sg) {
    m_sg = sg;
}

inline TradeManagerPtr ConditionBase::getTM() const {
    return m_tm;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::ConditionBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::ConditionPtr> : ostream_formatter {};
#endif

#endif /* CONDITIONBASE_H_ */
