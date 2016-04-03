/*
 * Ama.h
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#ifndef AMA_H_
#define AMA_H_

#include "../Indicator.h"

namespace hku {

/*
 * 佩里.J.考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
 * 参数： n: 计算均值的周期窗口，必须为大于2的整数
 *       fast_n: 快速趋势的周期，一般为2，不用改变
 *       slow_n: 对应慢速EMA线的N值，考夫曼一般设为30，不过当超过60左右该指标会收敛不会有太大的影响
 */
class Ama: public IndicatorImp {
    INDICATOR_IMP(Ama)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Ama();
    virtual ~Ama();
};

} /* namespace hku */
#endif /* AMA_H_ */
