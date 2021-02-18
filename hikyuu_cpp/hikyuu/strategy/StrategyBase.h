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
    StrategyBase();
    StrategyBase(const string& name);
    StrategyBase(const string& name, const string& config_file)
    : m_name(name), m_config_file(config_file) {}

    virtual ~StrategyBase() {}

    const string& name() const {
        return m_name;
    }

    void name(const string& name) {
        m_name = name;
    }

    const StrategyContext& context() const {
        return m_context;
    }

    void context(const StrategyContext& context) {
        m_context = context;
    }

    void start();

    virtual void init() = 0;
    virtual void on_bar() = 0;

private:
    string m_name;
    string m_config_file;
    StrategyContext m_context;
    TMPtr m_tm;
};

typedef shared_ptr<StrategyBase> StrategyPtr;

}  // namespace hku