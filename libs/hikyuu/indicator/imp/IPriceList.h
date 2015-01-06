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

class IPriceList: public IndicatorImp {
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IPriceList();
    IPriceList(const PriceList&);
    IPriceList(const PriceList&, size_t start, size_t end);
    IPriceList(const Indicator&, size_t result_num, size_t start, size_t end);
    IPriceList(price_t *, size_t);
    virtual ~IPriceList();

    virtual string name() const;
};


} /* namespace hku */
#endif /* IPRICELIST_H_ */
