/*
 * IKData.h
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#ifndef IKDATA_H_
#define IKDATA_H_

#include "../Indicator.h"

namespace hku {

class IKData: public IndicatorImp {
    INDICATOR_IMP(IKData)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IKData();
    IKData(const KData&, const string&);
    virtual ~IKData();
};

} /* namespace hku */
#endif /* IKDATA_H_ */
