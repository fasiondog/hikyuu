/*
 * StoplossBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef STOPLOSSBASE_H_
#define STOPLOSSBASE_H_

#include "../../KData.h"
// #include "../../utilities/Parameter.h"
#include "../../trade_manage/TradeManager.h"

namespace hku {

/**
 * Base class of the stop-loss / take-profit strategy
 * @details It is responsible for providing the expected stop-loss price of the current planned
 * trade to the system
 * @ingroup Stoploss
 */
class HKU_API StoplossBase : public enable_shared_from_this<StoplossBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    StoplossBase();
    explicit StoplossBase(const string& name);
    StoplossBase(const StoplossBase&) = default;
    virtual ~StoplossBase();

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    /** Set the trade management instance */
    void setTM(const TradeManagerPtr& tm);

    /** Get the trade management instance */
    TradeManagerPtr getTM() const;

    /** Set the trading object */
    void setTO(const KData& kdata);

    /** Get the trading object */
    KData getTO() const;

    /** Reset operation */
    void reset();

    typedef shared_ptr<StoplossBase> StoplossPtr;
    /** Clone operation */
    StoplossPtr clone();

    /**
     * Get the planned stop-loss price of the current expected trade (a buy); 0 is returned if there
     * is no stop-loss price.
     * It is used by the system to query the planned stop-loss price of the current trade from the
     * stop-loss strategy module before the trade is executed.
     * @param datetime trade time
     * @param price the planned buy price
     * @note Generally the algorithms of the stop-loss and the take-profit can be interchanged, but
     *       the getPrice of the stop-loss can be passed the price of the planned trade, for example
     *       taking 30% of the buy price as the stop-loss. The take-profit does not consider the
     *       passed price parameter, i.e. it regards price as 0.0. In fact, even for the stop-loss
     * it is not recommended to use the price parameter; for example 30% of the previous day's low
     *       price can be used as the stop-loss, then the price parameter does not need to be
     *       considered
     */
    virtual price_t getPrice(const Datetime& datetime, price_t price) = 0;

    /**
     * Get the planned stop-loss price of the current expected trade (a short sell); 0 is returned
     * if there is no stop-loss price. It is used by the system to query the planned stop-loss price
     * of the current trade from the stop-loss strategy module before the trade is executed.
     * @param datetime trade date
     * @param price the planned trade price
     * @note In the default implementation it returns the same result as getPrice
     */
    virtual price_t getShortPrice(const Datetime& datetime, price_t price) {
        return getPrice(datetime, price);
    }

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual StoplossPtr _clone() = 0;

    /** Interface for the subclass to initialize the calculation, it is called by setTO */
    virtual void _calculate() {};

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    bool m_is_python_object{false};
    string m_name;
    TradeManagerPtr m_tm;
    KData m_kdata;

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        // m_kdata is set temporarily when the system runs, it does not need to be serialized
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        // m_kdata is set temporarily when the system runs, it does not need to be serialized
        // ar & BOOST_SERIALIZATION_NVP(m_kdata);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(StoplossBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public StoplossBase {
 *     STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Stoploss
 */
#define STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(StoplossBase); \
    }
#else
#define STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define STOPLOSS_IMP(classname, str_name)     \
public:                                       \
    virtual StoplossPtr _clone() override {   \
        return std::make_shared<classname>(); \
    }                                         \
    virtual void _calculate() override;       \
    virtual price_t getPrice(const Datetime&, price_t) override;

/**
 * Client programs should all use this pointer type to operate the stop-loss strategy instance
 * @ingroup Stoploss
 */
typedef shared_ptr<StoplossBase> StoplossPtr;
typedef shared_ptr<StoplossBase> STPtr;
typedef shared_ptr<StoplossBase> TakeProfitPtr;
typedef shared_ptr<StoplossBase> TPPtr;

HKU_API std::ostream& operator<<(std::ostream& os, const StoplossBase&);
HKU_API std::ostream& operator<<(std::ostream& os, const StoplossPtr&);

inline const string& StoplossBase::name() const {
    return m_name;
}

inline void StoplossBase::name(const string& name) {
    m_name = name;
}

inline TradeManagerPtr StoplossBase::getTM() const {
    return m_tm;
}

inline void StoplossBase::setTM(const TradeManagerPtr& tm) {
    m_tm = tm;
}

inline KData StoplossBase::getTO() const {
    return m_kdata;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::StoplossBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::StoplossPtr> : ostream_formatter {};
#endif

#endif /* STOPLOSSBASE_H_ */
