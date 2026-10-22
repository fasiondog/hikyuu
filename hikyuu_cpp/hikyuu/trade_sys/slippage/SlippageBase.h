/*
 * SlippageBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef SLIPPAGEBASE_H_
#define SLIPPAGEBASE_H_

#include "../../KData.h"
#include "../../utilities/Parameter.h"

namespace hku {

/**
 * Base class of the slippage algorithm
 * @ingroup Slippage
 */
class HKU_API SlippageBase {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    SlippageBase();
    explicit SlippageBase(const string& name);
    SlippageBase(const SlippageBase&) = default;
    virtual ~SlippageBase() {}

    /** Set the trading object */
    void setTO(const KData& kdata);

    /** Get the trading object */
    KData getTO() const;

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    /** Reset operation */
    void reset();

    typedef shared_ptr<SlippageBase> SlippagePtr;
    /** Clone operation */
    SlippagePtr clone();

    /**
     * Get the buy price of the slippage model. This is a non-virtual wrapper that only calls the
     * subclass _getRealBuyPrice; the System part owns the price authority and validates the result
     * and clamps it into the current bar's [low, high] range before the trade.
     * @param datetime buy moment
     * @param planPrice planned buy price
     * @return the buy price of the slippage model
     */
    price_t getRealBuyPrice(const Datetime& datetime, price_t planPrice);

    /**
     * Get the sell price of the slippage model. See getRealBuyPrice.
     * @param datetime sell moment
     * @param planPrice planned sell price
     * @return the sell price of the slippage model
     */
    price_t getRealSellPrice(const Datetime& datetime, price_t planPrice);

    /**
     * Get the price used to cover a short position (buying back). See getRealBuyPrice.
     * @param datetime buy moment
     * @param planPrice planned buy price
     * @return the buy price of the slippage model for covering a short
     */
    price_t getRealBuyShortPrice(const Datetime& datetime, price_t planPrice);

    /**
     * Get the price used to open a short position (short selling). See getRealSellPrice.
     * @param datetime sell moment
     * @param planPrice planned sell price
     * @return the sell price of the slippage model for opening a short
     */
    price_t getRealSellShortPrice(const Datetime& datetime, price_t planPrice);

    /** Subclass clone interface */
    virtual SlippagePtr _clone() = 0;

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass calculation interface, it is called by setTO */
    virtual void _calculate() = 0;

    /**
     * Subclass interface: compute the raw buy price (without the day-range clamp).
     * Override this instead of getRealBuyPrice; the base class applies the clamp.
     * @param datetime buy moment
     * @param planPrice planned buy price
     * @return the raw buy price
     */
    virtual price_t _getRealBuyPrice(const Datetime& datetime, price_t planPrice) = 0;

    /**
     * Subclass interface: compute the raw sell price (without the day-range clamp).
     * @param datetime sell moment
     * @param planPrice planned sell price
     * @return the raw sell price
     */
    virtual price_t _getRealSellPrice(const Datetime& datetime, price_t planPrice) = 0;

    /**
     * Subclass interface: compute the raw price for covering a short position.
     * The default delegates to _getRealBuyPrice, so overriding is needed only when the short side
     * should slip differently from the long side.
     * @param datetime buy moment
     * @param planPrice planned buy price
     * @return the raw buy price for covering a short
     */
    virtual price_t _getRealBuyShortPrice(const Datetime& datetime, price_t planPrice);

    /**
     * Subclass interface: compute the raw price for opening a short position.
     * The default delegates to _getRealSellPrice.
     * @param datetime sell moment
     * @param planPrice planned sell price
     * @return the raw sell price for opening a short
     */
    virtual price_t _getRealSellShortPrice(const Datetime& datetime, price_t planPrice);

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

    /** Whether this component itself is defined in Python; subclasses with Python parts recurse */
    virtual bool hasPythonPart() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    KData m_kdata;
    bool m_is_python_object{false};
    bool m_calculated{false};  // It is for the calculation at setTO only

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
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SlippageBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public SlippageBase {
 *     SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Slippage
 */
#define SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SlippageBase); \
    }
#else
#define SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define SLIPPAGE_IMP(classname)                                           \
public:                                                                   \
    virtual SlippagePtr _clone() override {                               \
        return std::make_shared<classname>();                             \
    }                                                                     \
    virtual price_t _getRealBuyPrice(const Datetime&, price_t) override;  \
    virtual price_t _getRealSellPrice(const Datetime&, price_t) override; \
    virtual void _calculate() override;

// For RNG-based slippages: copy the source (so the cloned params/seed are carried over), then
// reset() to reseed the RNG from the (copied) seed. The base clone() then restores the common
// members including m_kdata and clears m_calculated, so the clone keeps the source's kdata and
// recalculates on the next setTO.
#define SLIPPAGE_IMP_RNG(classname)                                       \
public:                                                                   \
    virtual SlippagePtr _clone() override {                               \
        auto p = std::make_shared<classname>(*this);                      \
        p->reset();                                                       \
        return p;                                                         \
    }                                                                     \
    virtual price_t _getRealBuyPrice(const Datetime&, price_t) override;  \
    virtual price_t _getRealSellPrice(const Datetime&, price_t) override; \
    virtual void _calculate() override;

/**
 * Client programs should all use this pointer type to operate the slippage algorithm
 * @ingroup Slippage
 */
typedef shared_ptr<SlippageBase> SlippagePtr;
typedef shared_ptr<SlippageBase> SPPtr;

HKU_API std::ostream& operator<<(std::ostream&, const SlippageBase&);
HKU_API std::ostream& operator<<(std::ostream&, const SlippagePtr&);

inline const string& SlippageBase::name() const {
    return m_name;
}

inline void SlippageBase::name(const string& name) {
    m_name = name;
}

inline KData SlippageBase::getTO() const {
    return m_kdata;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SlippageBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::SlippagePtr> : ostream_formatter {};
#endif

#endif /* SLIPPAGEBASE_H_ */
