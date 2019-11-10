/*
 * IIntpart.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IINTPART_H_
#define INDICATOR_IMP_IINTPART_H_

#include "../Indicator.h"

namespace hku {

/*
 * 取整(绝对值减小取整，即取得数据的整数部分)
 */
class IIntpart : public IndicatorImp {
    INDICATOR_IMP(IIntpart)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIntpart();
    virtual ~IIntpart();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IINTPART_H_ */
