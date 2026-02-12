/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#include "FactorMeta.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const FactorMeta& factor) {
    string strip("  \n");
    os << "FactorMeta(";
    os << strip << "name: " << factor.m_data->name << strip << "ktype: " << factor.m_data->ktype
       << strip << "is_active: " << factor.m_data->is_active << strip
       << "create_at: " << factor.m_data->create_at.str() << strip
       << "update_at: " << factor.m_data->update_at.str() << strip
       << "formula: " << factor.m_data->ind << strip
       << "description: " << factor.m_data->description << ")";
    return os;
}

string FactorMeta::str() const {
    std::ostringstream os;
    os << *this;
    return os.str();
}

FactorMeta::FactorMeta() : m_data(make_shared<Data>()) {}

FactorMeta::FactorMeta(const string& name, const KQuery::KType& ktype, const string& desc,
                       const Indicator& ind)
: FactorMeta() {
    m_data->name = name;
    m_data->ktype = ktype;
    m_data->description = desc;
    m_data->ind = ind;
    m_data->create_at = Datetime::now();
    m_data->is_active = true;
}

FactorMeta::FactorMeta(const FactorMeta& other) : FactorMeta() {
    m_data = other.m_data;
}

FactorMeta::FactorMeta(const FactorMeta&& other) : FactorMeta() {
    m_data = std::move(other.m_data);
}

FactorMeta& FactorMeta::operator=(const FactorMeta& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data->name = other.m_data->name;
    m_data->ktype = other.m_data->ktype;
    m_data->description = other.m_data->description;
    m_data->ind = other.m_data->ind;
    m_data->create_at = other.m_data->create_at;
    m_data->update_at = other.m_data->update_at;
    m_data->is_active = other.m_data->is_active;
    return *this;
}

FactorMeta& FactorMeta::operator=(const FactorMeta&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data->name = std::move(other.m_data->name);
    m_data->ktype = std::move(other.m_data->ktype);
    m_data->description = std::move(other.m_data->description);
    m_data->ind = std::move(other.m_data->ind);
    m_data->create_at = std::move(other.m_data->create_at);
    m_data->update_at = std::move(other.m_data->update_at);
    m_data->is_active = other.m_data->is_active;
    other.m_data->is_active = false;
    return *this;
}

}  // namespace hku