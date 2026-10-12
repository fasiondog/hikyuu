/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ITIME_H_
#define INDICATOR_IMP_ITIME_H_

#include "../Indicator.h"

namespace hku {

/*
 * Implementation of the TDX time functions
 */
class ITime : public IndicatorImp {
    INDICATOR_IMP(ITime)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITime();
    virtual ~ITime() override;
    virtual void _checkParam(const string& name) const override;

private:
    DatetimeList _getDatetimeList(const Indicator& data) const;
    void _doCalculate(const DatetimeList& ds, size_t start_pos);
};

}  // namespace hku

#endif  // INDICATOR_IMP_ITIME_H_