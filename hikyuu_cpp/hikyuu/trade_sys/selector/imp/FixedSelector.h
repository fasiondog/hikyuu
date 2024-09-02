/*
 * FixedSelector.h
 *
 *  Created on: 2018年1月12日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SELECTOR_IMP_FIXEDSELECTOR_H_
#define TRADE_SYS_SELECTOR_IMP_FIXEDSELECTOR_H_

#include "../SelectorBase.h"

namespace hku {

class FixedSelector : public SelectorBase {
    SELECTOR_IMP(FixedSelector)
    SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedSelector();
    explicit FixedSelector(double weight);
    virtual ~FixedSelector();
};

} /* namespace hku */

#endif /* TRADE_SYS_SELECTOR_IMP_FIXEDSELECTOR_H_ */
