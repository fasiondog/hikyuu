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

    explicit InterruptFlag(bool initial) : m_flag(initial) {}

    InterruptFlag(const InterruptFlag& other)
    : m_flag(other.m_flag.load(std::memory_order_relaxed)) {}

    // 赋值运算符
    InterruptFlag& operator=(const InterruptFlag& other) {
        m_flag.store(other.m_flag.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }

    // 转换为 bool 类型
    operator bool() const {
        return m_flag.load(std::memory_order_relaxed);
    }

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