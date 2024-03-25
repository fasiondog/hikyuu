/*
 * IKData.h
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IKDATA_H_
#define INDICATOR_IMP_IKDATA_H_

#include "../Indicator.h"

namespace hku {

class IKData : public IndicatorImp {
    INDICATOR_IMP(IKData)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IKData();
    IKData(const KData&, const string&);
    virtual ~IKData();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IKDATA_H_ */
