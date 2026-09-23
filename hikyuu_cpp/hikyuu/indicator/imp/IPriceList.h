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

// Find the last Null<price_t> in the array and set all the preceding data to Null
class IPriceList : public IndicatorImp {
    INDICATOR_IMP(IPriceList)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IPriceList();
    IPriceList(const PriceList&, int discard);
    IPriceList(PriceList&&, int discard);
    IPriceList(size_t size, double value, int discard);
    virtual ~IPriceList() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IPRICELIST_H_ */
