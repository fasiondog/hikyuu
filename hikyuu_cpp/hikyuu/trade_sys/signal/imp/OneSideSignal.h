/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_IMP_ONESIDESIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_ONESIDESIGNAL_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

// 根据输入指标构建单边信号（单纯的只包含买入或卖出信号），如果指标值大于0，则加入信号
class OneSideSignal : public SignalBase {
public:
    OneSideSignal();
    OneSideSignal(const Indicator& ind, bool is_buy);
    virtual ~OneSideSignal();

    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override;
    virtual void _checkParam(const string& name) const override;

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

#endif /* TRADE_SYS_SIGNAL_IMP_ONESIDESIGNAL_H_ */
