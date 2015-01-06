/*
 * StdDeviation.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#ifndef STDDEVIATION_H_
#define STDDEVIATION_H_

#include "../Indicator.h"

namespace hku {

/*
 * 计算N周期内样本标准差
 * 参数： n: N日时间窗口
 * TODO      ma : 计算均值的函数原型
 *       link：均值参数联动标志，默认true
 *
 */
class StdDeviation: public hku::IndicatorImp {
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    StdDeviation();
    StdDeviation(int n);
    StdDeviation(const Indicator& data, int n);
    virtual ~StdDeviation();

    virtual string name() const;
    virtual IndicatorImpPtr operator()(const Indicator& ind);
};

} /* namespace hku */
#endif /* STDDEVIATION_H_ */
