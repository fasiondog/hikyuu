/*
 * SingleSignal2.h
 *
 *  Created on: 2016年4月16日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL2_H_
#define TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL2_H_

#include "../../../indicator/Operand.h"
#include "../SignalBase.h"

namespace hku {

class SingleSignal2: public SignalBase {
public:
    SingleSignal2();
    SingleSignal2(const Operand&);
    virtual ~SingleSignal2();

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

#endif /* TRADE_SYS_SIGNAL_IMP_SINGLESIGNAL2_H_ */
