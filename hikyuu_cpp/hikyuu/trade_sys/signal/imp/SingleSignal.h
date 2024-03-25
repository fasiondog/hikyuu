/*
 * SingleSignal.h
 *
 *  Created on: 2015年2月22日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

class SingleSignal : public SignalBase {
public:
    SingleSignal();
    explicit SingleSignal(const Indicator& ind);
    virtual ~SingleSignal();

    virtual void _checkParam(const string& name) const override;
    virtual SignalPtr _clone() override;
    virtual void _calculate() override;

private:
    Indicator m_ind;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
        ar& BOOST_SERIALIZATION_NVP(m_ind);
    }
#endif
};

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL_H_ */
