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
    explicit FactorMeta(const string& name, const Indicator& formula,
                        const KQuery::KType& ktype = KQuery::DAY, const string& brief = "",
                        const string& details = "");

    FactorMeta(const FactorMeta& other);
    FactorMeta(const FactorMeta&& other);
    virtual ~FactorMeta() = default;

    FactorMeta& operator=(const FactorMeta& other);
    FactorMeta& operator=(const FactorMeta&& other);

    //------------------------
    // 只读属性
    //------------------------

    const string& name() const noexcept {
        return m_data->name;
    }

    const string& ktype() const noexcept {
        return m_data->ktype;
    }

    Indicator formula() const noexcept {
        return m_data->formula.clone();
    }

    const Datetime& createAt() const noexcept {
        return m_data->create_at;
    }

    const Datetime& updateAt() const noexcept {
        return m_data->update_at;
    }

    //------------------------
    // 可读写属性
    //------------------------

    const string& brief() const noexcept {
        return m_data->brief;
    }

    void brief(const string& brief) noexcept {
        m_data->brief = brief;
    }

    const string& details() const noexcept {
        return m_data->details;
    }

    void details(const string& details) noexcept {
        m_data->details = details;
    }

    bool isActive() const noexcept {
        return m_data->is_active;
    }

    void isActive(bool flag) noexcept {
        m_data->is_active = flag;
    }

    //------------------------
    // 其他接口
    //------------------------

    Indicator getIndicator(const Stock&, const KQuery&) const;

    uint64_t hash() const noexcept;

    string str() const;

private:
    struct Data {
        string name;
        string ktype;
        string brief;
        string details;
        Datetime create_at;
        Datetime update_at;
        Indicator formula;
        bool is_active{false};
    };
    shared_ptr<Data> m_data;
};

HKU_API std::ostream& operator<<(std::ostream& os, const FactorMeta&);

}  // namespace hku

namespace std {
template <>
class hash<hku::FactorMeta> {
public:
    size_t operator()(hku::FactorMeta const& factor) const noexcept {
        return factor.hash();
    }
};
}  // namespace std