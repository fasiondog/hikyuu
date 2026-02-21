/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class HKU_API Factor final {
    friend HKU_API std::ostream& operator<<(std::ostream& os, const Factor&);

public:
    Factor();
    Factor(const string& name, const Indicator& formula, const KQuery::KType& ktype = KQuery::DAY,
           const string& brief = "", const string& details = "", bool need_persist = false);

    Factor(const Factor& other);
    Factor(Factor&& other);
    ~Factor() = default;

    Factor& operator=(const Factor& other);
    Factor& operator=(Factor&& other);

    //------------------------
    // 只读属性
    //------------------------

    const string& name() const noexcept {
        return m_data->name;
    }

    const string& ktype() const noexcept {
        return m_data->ktype;
    }

    Indicator formula() const {
        return m_data->formula.clone();
    }

    //------------------------
    // 可读写属性
    //------------------------

    const Datetime& createAt() const noexcept {
        return m_data->create_at;
    }

    void createAt(const Datetime& datetime) {
        HKU_CHECK(m_data == ms_null_factor_meta_data, "Can not be called when m_data is null!");
        m_data->create_at = datetime;
    }

    const Datetime& updateAt() const noexcept {
        return m_data->update_at;
    }

    void updateAt(const Datetime& datetime) {
        HKU_CHECK(m_data == ms_null_factor_meta_data, "Can not be called when m_data is null!");
        m_data->update_at = datetime;
    }

    bool needPersist() const noexcept {
        return m_data->need_persist;
    }

    void needPersist(bool flag) {
        HKU_CHECK(m_data == ms_null_factor_meta_data, "Can not be called when m_data is null!");
        m_data->need_persist = flag;
    }

    const string& brief() const noexcept {
        return m_data->brief;
    }

    void brief(const string& brief) {
        HKU_CHECK(m_data == ms_null_factor_meta_data, "Can not be called when m_data is null!");
        m_data->brief = brief;
    }

    const string& details() const noexcept {
        return m_data->details;
    }

    void details(const string& details) {
        HKU_CHECK(m_data == ms_null_factor_meta_data, "Can not be called when m_data is null!");
        m_data->details = details;
    }

    bool isActive() const noexcept {
        return m_data->is_active;
    }

    void isActive(bool flag) {
        HKU_CHECK(m_data == ms_null_factor_meta_data, "Can not be called when m_data is null!");
        m_data->is_active = flag;
    }

    //------------------------
    // 其他接口
    //------------------------

    bool isNull() const noexcept {
        return m_data == ms_null_factor_meta_data;
    }

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
        bool need_persist{false};
        bool is_active{false};
    };
    shared_ptr<Data> m_data;

private:
    static shared_ptr<Data> ms_null_factor_meta_data;
};

HKU_API std::ostream& operator<<(std::ostream& os, const Factor&);

}  // namespace hku

namespace std {
template <>
class hash<hku::Factor> {
public:
    size_t operator()(hku::Factor const& factor) const noexcept {
        return factor.hash();
    }
};
}  // namespace std