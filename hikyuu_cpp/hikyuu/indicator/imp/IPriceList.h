/*
 * IPriceList.h
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IPRICELIST_H_
#define INDICATOR_IMP_IPRICELIST_H_

#include "../Indicator.h"

namespace hku {

// 找到数组最后一个Null<price_t>，并将之前的数据全部置为Null
class IPriceList : public IndicatorImp {
    INDICATOR_IMP(IPriceList)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IPriceList();
    IPriceList(const PriceList&, int discard);
    virtual ~IPriceList();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IPRICELIST_H_ */
