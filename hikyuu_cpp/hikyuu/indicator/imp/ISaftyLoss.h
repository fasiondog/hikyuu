/*
 * SaftyLoss.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef ISAFTYLOSS_H_
#define ISAFTYLOSS_H_

#include "../Indicator.h"

namespace hku {

/*
 * 亚历山大.艾尔德安全地带止损
 * 参见《走进我的交易室》（2007年 地震出版社） 亚历山大.艾尔德(Alexander Elder) P202
 * 计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，
 *         得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日
 *         最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的
 *         上移，在上述结果的基础上再取起N日（一般为3天）内的最高值
 * 注意事项：返回结果中前（回溯周期宽度＋去最高值的宽度）个点是无效的
 * 参数： n1: 计算平均噪音的回溯时间窗口，默认为10天
 *       n2: 对初步止损线取n2日内的最高值，默认为3
 *       p: 噪音系数，默认为2
 */
class ISaftyLoss : public hku::IndicatorImp {
    INDICATOR_IMP(ISaftyLoss)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISaftyLoss();
    virtual ~ISaftyLoss();

    virtual void _checkParam(const string& name) const override;
    virtual void _dyn_calculate(const Indicator&) override;

private:
    void _dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2, double p);
};

} /* namespace hku */
#endif /* ISAFTYLOSS_H_ */
