/*
 * ITimeLine.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ITIMELINE_H_
#define INDICATOR_IMP_ITIMELINE_H_

#include "../Indicator.h"

namespace hku {

class ITimeLine : public IndicatorImp {
    INDICATOR_IMP(ITimeLine)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITimeLine();
    explicit ITimeLine(const KData&);
    virtual ~ITimeLine();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ITIMELINE_H_ */
