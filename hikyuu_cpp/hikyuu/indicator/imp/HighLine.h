/*
 * HighLine.h
 *
 *  Created on: 2016年4月1日
 *      Author: fasiondog
 */

#ifndef INDICATOR_IMP_HIGHLINE_H_
#define INDICATOR_IMP_HIGHLINE_H_

#include "../Indicator.h"

namespace hku {

/*
 * N日内最高价，一般使用最高价数据作为输入
 * 参数： n: N日时间窗口
 */
class HighLine: public IndicatorImp {
    INDICATOR_IMP(HighLine)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    HighLine();
    virtual ~HighLine();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_HIGHLINE_H_ */
