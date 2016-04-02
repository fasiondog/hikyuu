/*
 * FlexSignal.h
 *
 *  Created on: 2015年3月21日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_SIGNAL_IMP_FLEXSIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_FLEXSIGNAL_H_

#include "../SignalBase.h"

namespace hku {

class FlexSignal: public SignalBase {
public:
    FlexSignal();
    FlexSignal(const Indicator& ind);
    virtual ~FlexSignal();

    virtual SignalPtr _clone();
    virtual void _calculate();

private:
    Indicator m_ind;

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

#endif /* TRADE_SYS_SIGNAL_IMP_FLEXSIGNAL_H_ */
