/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/signal/SignalBase.h"

namespace hku {

class OperatorSignal : public SignalBase {
public:
    OperatorSignal();
    explicit OperatorSignal(const string& name);
    OperatorSignal(const string& name, const SignalPtr& sg1, const SignalPtr& sg2);
    virtual ~OperatorSignal();

    virtual void _reset() override;
    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override {}

protected:
    void sub_sg_calculate(SignalPtr& sg, const KData& kdata);

protected:
    SignalPtr m_sg1;
    SignalPtr m_sg2;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
        ar& BOOST_SERIALIZATION_NVP(m_sg1);
        ar& BOOST_SERIALIZATION_NVP(m_sg2);
    }
#endif
};

#define OPERATOR_SIGNAL_IMP(classname, name)                                                  \
public:                                                                                       \
    classname() : OperatorSignal(name) {}                                                     \
    classname(const SignalPtr& sg1, const SignalPtr& sg2) : OperatorSignal(name, sg1, sg2) {} \
    virtual ~classname() {}                                                                   \
    virtual SignalPtr _clone() override {                                                     \
        return make_shared<classname>(m_sg1, m_sg2);                                          \
    }                                                                                         \
    virtual void _calculate(const KData&) override;

#if HKU_SUPPORT_SERIALIZATION
#define OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION          \
private:                                                         \
    friend class boost::serialization::access;                   \
    template <class Archive>                                     \
    void serialize(Archive& ar, const unsigned int version) {    \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorSignal); \
    }
#else
#define OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

} /* namespace hku */
