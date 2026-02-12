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
    friend HKU_API std::ostream& operator<<(std::ostream& os, const FactorMeta&);

public:
    FactorMeta();
    explicit FactorMeta(const string& name, const KQuery::KType& ktype = KQuery::DAY,
                        const string& desc = "", const Indicator& ind = Indicator());

    FactorMeta(const FactorMeta& other);
    FactorMeta(const FactorMeta&& other);
    virtual ~FactorMeta() = default;

    FactorMeta& operator=(const FactorMeta& other);
    FactorMeta& operator=(const FactorMeta&& other);

    const string& name() const noexcept {
        return m_data->name;
    }

    const string& ktype() const noexcept {
        return m_data->ktype;
    }

    const string& description() const noexcept {
        return m_data->description;
    }

    void description(const string& description) noexcept {
        m_data->description = description;
    }

    const Indicator& indicator() const noexcept {
        return m_data->ind;
    }

    void indicator(const Indicator& ind) noexcept {
        m_data->ind = ind;
    }

    const Datetime& createAt() const noexcept {
        return m_data->create_at;
    }

    const Datetime& updateAt() const noexcept {
        return m_data->update_at;
    }

    bool isActive() const noexcept {
        return m_data->is_active;
    }

    void isActive(bool flag) noexcept {
        m_data->is_active = flag;
    }

    string str() const;

private:
    struct Data {
        string name;
        string ktype;
        string description;
        Datetime create_at;
        Datetime update_at;
        Indicator ind;
        bool is_active{false};
    };
    shared_ptr<Data> m_data;
};

HKU_API std::ostream& operator<<(std::ostream& os, const FactorMeta&);

}  // namespace hku