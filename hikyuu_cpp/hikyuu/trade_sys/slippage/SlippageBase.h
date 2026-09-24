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
class HKU_API SlippageBase : public enable_shared_from_this<SlippageBase> {
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
     * Calculate the actual buy price
     * @param datetime buy moment
     * @param planPrice planned buy price
     * @return the actual buy price
     */
    virtual price_t getRealBuyPrice(const Datetime& datetime, price_t planPrice) = 0;

    /**
     * Calculate the actual sell price
     * @param datetime sell moment
     * @param planPrice planned sell price
     * @return the actual sell price
     */
    virtual price_t getRealSellPrice(const Datetime& datetime, price_t planPrice) = 0;

    /** Subclass clone interface */
    virtual SlippagePtr _clone() = 0;

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass calculation interface, it is called by setTO */
    virtual void _calculate() = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    KData m_kdata;
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

#define SLIPPAGE_IMP(classname)                                          \
public:                                                                  \
    virtual SlippagePtr _clone() override {                              \
        return std::make_shared<classname>();                            \
    }                                                                    \
    virtual price_t getRealBuyPrice(const Datetime&, price_t) override;  \
    virtual price_t getRealSellPrice(const Datetime&, price_t) override; \
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
