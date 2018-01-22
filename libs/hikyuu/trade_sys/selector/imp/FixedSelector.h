/*
 * FixedSelector.h
 *
 *  Created on: 2018年1月12日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SELECTOR_IMP_FIXEDSELECTOR_H_
#define TRADE_SYS_SELECTOR_IMP_FIXEDSELECTOR_H_

#include "../SelectorBase.h"

namespace hku {

class FixedSelector: public SelectorBase {
    SELECTOR_IMP(FixedSelector);

public:
    FixedSelector();
    virtual ~FixedSelector();
};

} /* namespace hku */

#endif /* TRADE_SYS_SELECTOR_IMP_FIXEDSELECTOR_H_ */
