/*
 * BandSignal.h
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_IMP_BANDSIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_BANDSIGNAL_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

class BandSignal : public SignalBase {
public:
    BandSignal();
    BandSignal(const Indicator& sig, price_t lower, price_t upper);
    virtual ~BandSignal();

    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override;

private:
    Indicator m_ind;
    price_t m_lower = 0.0, m_upper = 0.0;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
        ar& BOOST_SERIALIZATION_NVP(m_ind);
        ar& BOOST_SERIALIZATION_NVP(m_lower);
        ar& BOOST_SERIALIZATION_NVP(m_upper);
    }
#endif
};
}  // namespace hku

#endif