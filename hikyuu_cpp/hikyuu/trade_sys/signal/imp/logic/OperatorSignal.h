/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include <functional>

#include "hikyuu/trade_sys/signal/SignalBase.h"

namespace hku {

class OperatorSignal : public SignalBase {
public:
    OperatorSignal();
    explicit OperatorSignal(const string& name);
    OperatorSignal(const string& name, const SignalPtr& sg1, const SignalPtr& sg2);
    virtual ~OperatorSignal();

    virtual void _reset() override final;

    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override {}
    virtual void setCycleDates(const DatetimeList& dates) override;

    virtual bool hasPythonPart() const noexcept override {
        return m_is_python_object || (m_sg1 && m_sg1->hasPythonPart()) ||
               (m_sg2 && m_sg2->hasPythonPart());
    }

protected:
    void sub_sg_calculate(SignalPtr& sg, const KData& kdata);

    typedef std::function<void(const Datetime&, double, double, double, double)> MergedEmitFunc;

    /**
     * Merge-join calculation over the sub-signal tables: the emit callback is called only at the
     * datetimes where any sub-signal table has an entry and only for real bars of kdata
     * @param kdata the trading object
     * @param emit the combine callback as (datetime, buy1, sell1, buy2, sell2); a value is 0.0
     *             when the table has no entry there or the sub-signal is null
     */
    void sub_merged_calculate(const KData& kdata, const MergedEmitFunc& emit);

protected:
    SignalPtr m_sg1;
    SignalPtr m_sg2;

//============================================
// Serialization support
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
    virtual ~classname() override {}                                                          \
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
