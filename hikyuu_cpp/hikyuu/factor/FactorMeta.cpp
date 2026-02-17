/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#include "hikyuu/indicator/imp/IPriceList.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "FactorMeta.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const FactorMeta& factor) {
    string strip("  \n");
    os << "FactorMeta(";
    os << strip << "name: " << factor.m_data->name << strip << "ktype: " << factor.m_data->ktype
       << strip << "is_active: " << factor.m_data->is_active << strip
       << "create_at: " << factor.m_data->create_at.str() << strip
       << "update_at: " << factor.m_data->update_at.str() << strip
       << "formula: " << factor.m_data->formula() << strip << "brief: " << factor.m_data->brief
       << strip << "detail: " << factor.m_data->details << ")";
    return os;
}

string FactorMeta::str() const {
    std::ostringstream os;
    os << *this;
    return os.str();
}

static bool isValidName(const string& name) {
    // 名称规则: 英文字母、数字、_ 组成，且首字母不能为数字

    // 检查名称是否为空
    if (name.empty()) {
        return false;
    }

    // 检查首字符是否为字母或下划线
    if (!isalpha(name[0]) && name[0] != '_') {
        return false;
    }

    // 检查其余字符是否都是字母、数字或下划线
    for (size_t i = 1; i < name.length(); ++i) {
        if (!isalnum(name[i]) && name[i] != '_') {
            return false;
        }
    }

    return true;
}

FactorMeta::FactorMeta() : m_data(make_shared<Data>()) {}

FactorMeta::FactorMeta(const string& name, const Indicator& formula, const KQuery::KType& ktype,
                       const string& brief, const string& details)
: FactorMeta() {
    HKU_CHECK(isValidName(name),
              htr("Illegal name! Naming rules: Only letters, digits and underscores (_) allowed; "
                  "cannot start with a digit."));
    Indicator ind = formula.clone();
    ind.setContext(KData());
    IndicatorImp* imp_ptr = ind.getImp().get();
    HKU_CHECK(imp_ptr, "Invalid formula indicator!");

    IPriceList* tmp_ptr = dynamic_cast<IPriceList*>(imp_ptr);
    if (tmp_ptr) {
        ind = PRICELIST();
    }

    m_data->name = name;
    m_data->ktype = ktype;
    m_data->brief = brief;
    m_data->details = details;
    m_data->formula = ind;
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
    m_data->brief = other.m_data->brief;
    m_data->details = other.m_data->details;
    m_data->formula = other.m_data->formula;
    m_data->create_at = other.m_data->create_at;
    m_data->update_at = other.m_data->update_at;
    m_data->is_active = other.m_data->is_active;
    return *this;
}

FactorMeta& FactorMeta::operator=(const FactorMeta&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data->name = std::move(other.m_data->name);
    m_data->ktype = std::move(other.m_data->ktype);
    m_data->brief = std::move(other.m_data->brief);
    m_data->details = std::move(other.m_data->details);
    m_data->formula = std::move(other.m_data->formula);
    m_data->create_at = std::move(other.m_data->create_at);
    m_data->update_at = std::move(other.m_data->update_at);
    m_data->is_active = other.m_data->is_active;
    other.m_data->is_active = false;
    return *this;
}

}  // namespace hku