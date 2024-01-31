/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

class SignalSelector : public SelectorBase {
    SELECTOR_IMP(SignalSelector)
    SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    SignalSelector();
    virtual ~SignalSelector();

    virtual void _reset() override {
        m_sys_dict_on_open.clear();
        m_sys_dict_on_close.clear();
    }

private:
    unordered_map<Datetime, SystemList> m_sys_dict_on_open;
    unordered_map<Datetime, SystemList> m_sys_dict_on_close;
};

}  // namespace hku