/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#pragma once

#include "../DataType.h"
#include "../StrategyContext.h"
#include "../utilities/Parameter.h"
#include "../trade_sys/portfolio/Portfolio.h"

namespace hku {

class HKU_API StrategyBase {
    PARAMETER_SUPPORT

public:
    StrategyBase() {}
    virtual ~StrategyBase() {}

    const string& name() const {
        return m_name;
    }

    void name(const string& name) {
        m_name = name;
    }

    // virtual void start() = 0;
    virtual void on_bar() = 0;

private:
    string m_name;
    StrategyContext m_context;
    TMPtr m_tm;
};

typedef shared_ptr<StrategyBase> StrategyPtr;

}  // namespace hku