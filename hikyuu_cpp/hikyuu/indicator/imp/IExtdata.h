/*
 * IExtdata.h
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IRPS_H_
#define INDICATOR_IMP_IRPS_H_

#include "../Indicator.h"
#include <hdf5.h>
#include <vector>
namespace hku {

class IExtdata : public IndicatorImp {
    INDICATOR_IMP(IExtdata)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    string filepath;
    IExtdata();
    virtual ~IExtdata() = default;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IRPS_H_ */
