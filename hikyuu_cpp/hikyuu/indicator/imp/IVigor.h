/*
 * IVigor.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef IVIGOR_H_
#define IVIGOR_H_

#include "../Indicator.h"

namespace hku {

/*
 * 亚历山大.艾尔德力度指数
 * 参见《走进我的交易室》（2007年 地震出版社） (Alexander Elder) P131
 * 计算公式：（收盘价今－收盘价昨）＊成交量今
 * n: 用于EMA平滑的周期窗口，必须为大于0的整数
 */
class IVigor : public IndicatorImp {
    INDICATOR_IMP(IVigor)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IVigor();
    explicit IVigor(int n);
    virtual ~IVigor();
};

} /* namespace hku */
#endif /* IVIGOR_H_ */
