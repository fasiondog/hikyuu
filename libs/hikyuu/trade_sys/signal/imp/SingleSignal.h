/*
 * SingleSignal.h
 *
 *  Created on: 2015年2月22日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL_H_

#include "../../../indicator/Operand.h"
#include "../SignalBase.h"

namespace hku {

class SingleSignal: public SignalBase {
public:
    SingleSignal();
    SingleSignal(const Operand& ind);
    virtual ~SingleSignal();

    virtual SignalPtr _clone();
    virtual void _calculate();

private:
    Operand m_ind;

    //============================================
    // 序列化支持
    //============================================
    #if HKU_SUPPORT_SERIALIZATION
        friend class boost::serialization::access; \
        template<class Archive> \
        void serialize(Archive & ar, const unsigned int version) { \
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase); \
            ar & BOOST_SERIALIZATION_NVP(m_ind);
        }
    #endif
};

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL_H_ */
