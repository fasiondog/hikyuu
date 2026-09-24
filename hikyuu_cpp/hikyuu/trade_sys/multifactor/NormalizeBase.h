/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/Parameter.h"

namespace hku {

/**
 * Base class of the time cross-section data normalization
 */
class HKU_API NormalizeBase {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    NormalizeBase() = default;
    NormalizeBase(const string& name) : m_name(name) {}

    NormalizeBase(const NormalizeBase& other) : m_params(other.m_params), m_name(other.m_name) {}

    virtual ~NormalizeBase() = default;

    /** Get the name */
    const string& name() const {
        return m_name;
    }

    /** Set the name */
    void name(const string& name) {
        m_name = name;
    }

    typedef std::shared_ptr<NormalizeBase> NormPtr;
    NormPtr clone();

    virtual NormPtr _clone() = 0;

    virtual PriceList normalize(const PriceList& data) = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

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
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NormalizeBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public NormalizeBase {
 *     NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MultiFactor
 */
#define NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                        \
    friend class boost::serialization::access;                  \
    template <class Archive>                                    \
    void serialize(Archive& ar, const unsigned int version) {   \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(NormalizeBase); \
    }
#else
#define NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef std::shared_ptr<NormalizeBase> NormPtr;
typedef std::shared_ptr<NormalizeBase> NormalizePtr;

#define NORMALIZE_IMP(classname)              \
public:                                       \
    virtual NormalizePtr _clone() override {  \
        return std::make_shared<classname>(); \
    }                                         \
    PriceList normalize(const PriceList& data) override;

HKU_API std::ostream& operator<<(std::ostream&, const NormalizeBase&);
HKU_API std::ostream& operator<<(std::ostream&, const NormalizePtr&);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::NormalizeBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::NormalizePtr> : ostream_formatter {};
#endif