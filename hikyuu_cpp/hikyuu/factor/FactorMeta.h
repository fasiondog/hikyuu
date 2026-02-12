/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class HKU_API FactorMeta {
public:
    FactorMeta() = default;
    virtual ~FactorMeta() = default;

    const string& name() const noexcept {
        return m_name;
    }

    void name(const string& name) noexcept {
        m_name = name;
    }

    const string& ktype() const noexcept {
        return m_ktype;
    }

    void ktype(const string& ktype) noexcept {
        m_ktype = ktype;
    }

    const string& description() const noexcept {
        return m_description;
    }

    void description(const string& description) noexcept {
        m_description = description;
    }

    const Indicator& indicator() const noexcept {
        return m_ind;
    }

    void indicator(const Indicator& ind) noexcept {
        m_ind = ind;
    }

    const Datetime& createAt() const noexcept {
        return m_create_at;
    }

    void createAt(const Datetime& datetime) noexcept {
        m_create_at = datetime;
    }

    const Datetime& updateAt() const noexcept {
        return m_update_at;
    }

    void updateAt(const Datetime& datetime) noexcept {
        m_update_at = datetime;
    }

    bool isActive() const noexcept {
        return m_is_active;
    }

    void isActive(bool flag) noexcept {
        m_is_active = flag;
    }

    bool saved() const noexcept {
        return m_saved;
    }

private:
    string m_name;
    string m_ktype;
    string m_description;
    Datetime m_create_at;
    Datetime m_update_at;
    Indicator m_ind;
    bool m_is_active{false};
    bool m_saved{false};
};

}  // namespace hku