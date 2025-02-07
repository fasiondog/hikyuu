/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/signal/SignalBase.h"

namespace hku {

class AddSignal : public SignalBase {
public:
    AddSignal();
    AddSignal(const SignalPtr& sg1, const SignalPtr& sg2);
    virtual ~AddSignal();

    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override;

private:
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

} /* namespace hku */
