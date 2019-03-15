/*
 * IPriceList.h
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#ifndef IPRICELIST_H_
#define IPRICELIST_H_

#include "../Indicator.h"

namespace hku {

//找到数组最后一个Null<price_t>，并将之前的数据全部置为Null
class IPriceList: public IndicatorImp {
    INDICATOR_IMP(IPriceList)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IPriceList();
    IPriceList(const PriceList&, int discard);
    IPriceList(price_t *, size_t);
    virtual ~IPriceList();
};


} /* namespace hku */
#endif /* IPRICELIST_H_ */
