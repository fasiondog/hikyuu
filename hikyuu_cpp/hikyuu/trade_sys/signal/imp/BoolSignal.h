/*
 * BoolSignal.h
 *
 *  Created on: 2017年7月2日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_IMP_BOOLSIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_BOOLSIGNAL_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

class BoolSignal : public SignalBase {
public:
    BoolSignal();
    BoolSignal(const Indicator& buy, const Indicator& sell);
    virtual ~BoolSignal();

    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override;

private:
    Indicator m_bool_buy;
    Indicator m_bool_sell;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
        ar& BOOST_SERIALIZATION_NVP(m_bool_buy);
        ar& BOOST_SERIALIZATION_NVP(m_bool_sell);
    }
#endif
};

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_IMP_BOOLSIGNAL_H_ */
