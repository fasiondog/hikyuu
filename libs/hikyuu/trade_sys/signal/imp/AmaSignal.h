/*
 * AmaSigal.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#ifndef AMASIGNAL_H_
#define AMASIGNAL_H_

#include "../SignalBase.h"

namespace hku {

class AmaSignal: public SignalBase {
    SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    AmaSignal(int n = 10, int fast_n = 2, int slow_n = 30,
            int filter_n = 20, double filter_p = 0.1,
            const string& kpart = "CLOSE");
    virtual ~AmaSignal();

    virtual void _reset();
    virtual SignalPtr _clone();
    virtual void _calculate();
};

} /* namespace hku */
#endif /* AMASIGNAL_H_ */
