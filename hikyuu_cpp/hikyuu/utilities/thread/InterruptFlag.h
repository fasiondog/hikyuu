/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/07/13
 *      Author: fasiondog
 */

#pragma once

#include <atomic>

namespace hku {

class InterruptFlag {
public:
    InterruptFlag() : m_flag(false) {}

    void set() {
        m_flag = true;
    }

    bool isSet() const {
        return m_flag;
    }

private:
    std::atomic_bool m_flag;
};

}  // namespace hku