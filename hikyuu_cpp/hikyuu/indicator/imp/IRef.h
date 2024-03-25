/*
 * IRef.h
 *
 *  Created on: 2015年3月21日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_RIGHTSHIFT_H_
#define INDICATOR_IMP_RIGHTSHIFT_H_

#include "../Indicator.h"

namespace hku {

/*
 * REF 向前引用 （即右移）
 * 引用若干周期前的数据。
 * 用法：　REF(X，A)　引用A周期前的X值。
 * 例如：　REF(CLOSE，1)　表示上一周期的收盘价，在日线上就是昨收。
 */
class IRef : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IRef)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRef();
    virtual ~IRef();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_RIGHTSHIFT_H_ */
