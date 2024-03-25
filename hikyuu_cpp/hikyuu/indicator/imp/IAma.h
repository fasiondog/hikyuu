/*
 * Ama.h
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#pragma once
#ifndef IAMA_H_
#define IAMA_H_

#include "../Indicator.h"

namespace hku {

/*
 * 佩里.J.考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
 * 参数： n: 计算均值的周期窗口，必须为大于2的整数
 *       fast_n: 快速趋势的周期，一般为2，不用改变
 *       slow_n: 对应慢速EMA线的N值，考夫曼一般设为30，不过当超过60左右该指标会收敛不会有太大的影响
 */
class IAma : public IndicatorImp {
    INDICATOR_IMP(IAma)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAma();
    virtual ~IAma();

    virtual void _checkParam(const string& name) const override;
    virtual void _dyn_calculate(const Indicator&) override;

private:
    void _dyn_one_circle(const Indicator& ind, size_t curPos, int n, int fast_n, int slow_n);
};

} /* namespace hku */
#endif /* IAMA_H_ */
