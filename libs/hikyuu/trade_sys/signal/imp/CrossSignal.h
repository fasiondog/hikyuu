/*
 * CrossSignal.h
 *
 *  Created on: 2015年2月20日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SIGNAL_IMP_CROSSSIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_CROSSSIGNAL_H_

#include "../../../indicator/Operand.h"
#include "../SignalBase.h"

namespace hku {

class CrossSignal: public SignalBase {
public:
    CrossSignal();
    CrossSignal(const Operand& fast, const Operand& slow, const string& kpart);
    virtual ~CrossSignal();

    virtual SignalPtr _clone();
    virtual void _calculate();

private:
    Operand m_fast;
    Operand m_slow;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase); \
        ar & BOOST_SERIALIZATION_NVP(m_fast);
        ar & BOOST_SERIALIZATION_NVP(m_slow);
    }
#endif
};

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_IMP_CROSSSIGNAL_H_ */
