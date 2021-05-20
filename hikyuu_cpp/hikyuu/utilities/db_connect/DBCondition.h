/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-20
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <type_traits>
#include <fmt/format.h>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

class HKU_API DBCondition {
public:
    DBCondition() {}

    DBCondition(const std::string& field, const std::string& val)
    : m_condition(fmt::format(R"({}="{}")", field, val)) {}

    DBCondition(const std::string& field, int val)
    : m_condition(fmt::format(R"({}={})", field, val)) {}

    DBCondition(const std::string& field, unsigned int val)
    : m_condition(fmt::format(R"({}={})", field, val)) {}

    DBCondition(const std::string& field, int64_t val)
    : m_condition(fmt::format(R"({}={})", field, val)) {}

    DBCondition(const std::string& field, uint64_t val)
    : m_condition(fmt::format(R"({}={})", field, val)) {}

    DBCondition(const std::string& field, float val)
    : m_condition(fmt::format(R"({}={})", field, val)) {}

    DBCondition(const std::string& field, double val)
    : m_condition(fmt::format(R"({}={})", field, val)) {}

    DBCondition& operator&(const DBCondition& other);
    DBCondition& operator|(const DBCondition& other);

    enum ORDERBY { ASC, DESC };

    void orderBy(const std::string& field, ORDERBY order) {
        m_condition = order == ORDERBY::ASC
                        ? fmt::format("{} order by {} ASC", m_condition, field)
                        : fmt::format("{} order by {} DESC", m_condition, field);
    }

    const std::string& str() const {
        return m_condition;
    }

private:
    std::string m_condition;
};

}  // namespace hku