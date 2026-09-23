/*
 * SignalBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef SIGNALBASE_H_
#define SIGNALBASE_H_

#include <set>
#include "../../KData.h"
#include "../../utilities/Parameter.h"
#include "../../trade_manage/TradeManager.h"
#include "../../serialization/Datetime_serialization.h"

namespace hku {

/**
 * Base class of the signal generator
 * @ingroup Signal
 */
class HKU_API SignalBase : public enable_shared_from_this<SignalBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    SignalBase();
    explicit SignalBase(const string& name);
    SignalBase(const SignalBase&) = default;
    virtual ~SignalBase();

    /**
     * Whether a buy is possible at the given moment
     * @param datetime the given moment
     * @return true a buy is possible | false a buy is not possible
     */
    bool shouldBuy(const Datetime& datetime) const;

    /**
     * Whether a sell is possible at the given moment
     * @param datetime the given moment
     * @return true a sell is possible | false a sell is not possible
     */
    bool shouldSell(const Datetime& datetime) const;

    /**
     * Get the buy signal value of the given moment; a value less than or equal to 0 means there is
     * no buy signal
     * @param datetime
     * @return double
     */
    double getBuyValue(const Datetime& datetime) const;

    /**
     * Get the sell signal value of the given moment; a value greater than or equal to 0 means there
     * is no sell signal
     * @param datetime
     * @return double
     */
    double getSellValue(const Datetime& datetime) const;

    double getValue(const Datetime& datetime) const;

    /**
     * Whether a buy is possible at the next moment, equivalent to whether the last moment indicates
     * a buy
     */
    bool nextTimeShouldBuy() const;

    /**
     * Whether a sell is possible at the next moment, equivalent to whether the last moment
     * indicates a sell
     */
    bool nextTimeShouldSell() const;

    /** Get the date list of all the buy indications */
    DatetimeList getBuySignal() const;

    /** Get the date list of all the sell indications */
    DatetimeList getSellSignal() const;

    void _addSignal(const Datetime& datetime, double value);

    /**
     * Add a buy signal, it is called in _calculate
     * @param datetime the date when the buy signal occurs
     * @param value signal value, 1.0 by default; it must be greater than 0, otherwise an exception
     * is thrown
     */
    void _addBuySignal(const Datetime& datetime, double value = 1.0);

    /**
     * Add a sell signal, it is called in _calculate
     * @param datetime the date when the sell signal occurs
     * @param value signal value, -1.0 by default; it must be less than 0, otherwise an exception is
     *              thrown
     */
    void _addSellSignal(const Datetime& datetime, double value = -1.0);

    /**
     * Set the trading object, it refers to the K-line data
     * @param kdata the given trading object
     */
    void setTO(const KData& kdata);

    /**
     * Get the trading object
     * @return the trading object (KData)
     */
    const KData& getTO() const;

    void startCycle(const Datetime& start, const Datetime& end);
    const Datetime& getCycleStart() const;
    const Datetime& getCycleEnd() const;

    /** Reset operation */
    void reset();

    typedef shared_ptr<SignalBase> SignalPtr;
    /** Clone operation */
    SignalPtr clone();

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual SignalPtr _clone() = 0;

    /** Subclass calculation interface, it is called in setTO */
    virtual void _calculate(const KData&) = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

private:
    void initParam();

protected:
    string m_name;
    KData m_kdata;
    bool m_is_python_object{false};
    bool m_calculated{false};  // It is for the calculation at setTO only

    /* Long positions */
    bool m_hold_long;
    /* Short positions */
    bool m_hold_short;

    // A map is used for the storage, so that the order can be kept when getting
    std::map<Datetime, double> m_buySig;
    std::map<Datetime, double> m_sellSig;

    Datetime m_cycle_start;
    Datetime m_cycle_end;

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
        ar& BOOST_SERIALIZATION_NVP(m_hold_long);
        ar& BOOST_SERIALIZATION_NVP(m_hold_short);
        ar& BOOST_SERIALIZATION_NVP(m_buySig);
        ar& BOOST_SERIALIZATION_NVP(m_sellSig);
        // m_kdata is set temporarily when the system runs, it does not need to be serialized
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
        // ar & BOOST_SERIALIZATION_NVP(m_calculated);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_hold_long);
        ar& BOOST_SERIALIZATION_NVP(m_hold_short);
        ar& BOOST_SERIALIZATION_NVP(m_buySig);
        ar& BOOST_SERIALIZATION_NVP(m_sellSig);
        // m_kdata is set temporarily when the system runs, it does not need to be serialized
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
        // ar & BOOST_SERIALIZATION_NVP(m_calculated);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SignalBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public SignalBase {
 *     SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Signal
 */
#define SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION                \
private:                                                      \
    friend class boost::serialization::access;                \
    template <class Archive>                                  \
    void serialize(Archive& ar, const unsigned int version) { \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);  \
    }
#else
#define SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define SIGNAL_IMP(classname)                 \
public:                                       \
    virtual SignalPtr _clone() override {     \
        return std::make_shared<classname>(); \
    }                                         \
    virtual void _calculate(const KData&) override;

/**
 * Client programs should all use this pointer type to operate the signal generator
 * @ingroup Signal
 */
typedef shared_ptr<SignalBase> SignalPtr;
typedef shared_ptr<SignalBase> SGPtr;

HKU_API std::ostream& operator<<(std::ostream&, const SignalBase&);
HKU_API std::ostream& operator<<(std::ostream&, const SignalPtr&);

inline const KData& SignalBase::getTO() const {
    return m_kdata;
}

inline const string& SignalBase::name() const {
    return m_name;
}

inline void SignalBase::name(const string& name) {
    m_name = name;
}

inline bool SignalBase::shouldBuy(const Datetime& datetime) const {
    return m_buySig.count(datetime) ? true : false;
}

inline bool SignalBase::shouldSell(const Datetime& datetime) const {
    return m_sellSig.count(datetime) ? true : false;
}

inline const Datetime& SignalBase::getCycleStart() const {
    return m_cycle_start;
}

inline const Datetime& SignalBase::getCycleEnd() const {
    return m_cycle_end;
}

inline double SignalBase::getValue(const Datetime& datetime) const {
    return getBuyValue(datetime) + getSellValue(datetime);
}

inline void SignalBase::_addBuySignal(const Datetime& datetime, double value) {
    HKU_IF_RETURN(std::isnan(value), void());
    HKU_CHECK(value > 0.0, "buy value muse be > 0", value);
    _addSignal(datetime, value);
}

inline void SignalBase::_addSellSignal(const Datetime& datetime, double value) {
    HKU_IF_RETURN(std::isnan(value), void());
    HKU_CHECK(value < 0.0, "sell value muse be > 0", value);
    _addSignal(datetime, value);
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SignalBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::SignalPtr> : ostream_formatter {};
#endif

#endif /* SIGNALBASE_H_ */
