/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/signal/SignalBase.h"

namespace hku {

class OperatorValueSignal : public SignalBase {
public:
    OperatorValueSignal();
    explicit OperatorValueSignal(const string& name);
    OperatorValueSignal(const string& name, const SignalPtr& sg, double value, int mode = 0);
    virtual ~OperatorValueSignal();

    virtual void _reset() override;
    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override {}

protected:
    double m_value{0.0};
    SignalPtr m_sg;
    int m_mode{0};  // 仅对-、/有效，0：(sg, value), 1: (value, sg)

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
        ar& BOOST_SERIALIZATION_NVP(m_sg);
        ar& BOOST_SERIALIZATION_NVP(m_value);
        ar& BOOST_SERIALIZATION_NVP(m_mode);
    }
#endif
};

#define OPERATOR_SIGNAL_IMP(classname, name)                                               \
public:                                                                                    \
    classname() : OperatorValueSignal(name) {}                                             \
    classname(const SignalPtr& sg, double value) : OperatorValueSignal(name, sg, value) {} \
    virtual ~classname() {}                                                                \
    virtual SignalPtr _clone() override {                                                  \
        return make_shared<classname>(m_sg, m_value);                                      \
    }                                                                                      \
    virtual void _calculate(const KData&) override;

#if HKU_SUPPORT_SERIALIZATION
#define OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                              \
    friend class boost::serialization::access;                        \
    template <class Archive>                                          \
    void serialize(Archive& ar, const unsigned int version) {         \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);          \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorValueSignal); \
    }
#else
#define OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

} /* namespace hku */
