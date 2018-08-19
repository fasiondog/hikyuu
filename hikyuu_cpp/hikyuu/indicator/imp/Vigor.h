/*
 * Vigor.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#ifndef VIGOR_H_
#define VIGOR_H_

#include "../Indicator.h"

namespace hku {

/*
 * 亚历山大.艾尔德力度指数
 * 参见《走进我的交易室》（2007年 地震出版社） (Alexander Elder) P131
 * 计算公式：（收盘价今－收盘价昨）＊成交量今
 * n: 用于EMA平滑的周期窗口，必须为大于0的整数
 */
class Vigor: public IndicatorImp {
    INDICATOR_IMP(Vigor)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Vigor();
    Vigor(const KData& kdata, int n);
    virtual ~Vigor();
};

} /* namespace hku */
#endif /* VIGOR_H_ */
