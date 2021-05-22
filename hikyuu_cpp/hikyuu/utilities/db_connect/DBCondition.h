/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-20
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include "../../Log.h"

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

struct ASC {
    ASC(const std::string& name) : name(name) {}
    std::string name;
};

struct DESC {
    DESC(const std::string& name) : name(name) {}
    std::string name;
};

class HKU_API DBCondition {
public:
    DBCondition() {}

    DBCondition(const std::string& cond) : m_condition(cond) {}

    DBCondition& operator&(const DBCondition& other);
    DBCondition& operator|(const DBCondition& other);

    enum ORDERBY { ORDER_ASC, ORDER_DESC };

    void orderBy(const std::string& field, ORDERBY order) {
        m_condition = order == ORDERBY::ORDER_ASC
                        ? fmt::format("{} order by {} ASC", m_condition, field)
                        : fmt::format("{} order by {} DESC", m_condition, field);
    }

    DBCondition& operator+(const ASC& asc) {
        orderBy(asc.name, ORDER_ASC);
        return *this;
    }

    DBCondition& operator+(const DESC& desc) {
        orderBy(desc.name, ORDER_ASC);
        return *this;
    }

    const std::string& str() const {
        return m_condition;
    }

private:
    std::string m_condition;
};

struct Field {
    Field(const std::string& name) : name(name) {}

    // in 和 not_in 不支持 字符串，一般不会用到 in ("stra", "strb") 的 SQL 操作
    template <typename T>
    DBCondition in(const std::vector<T>& vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        return DBCondition(fmt::format("({} in ({}))", name, fmt::join(vals, ",")));
    }

    template <>
    DBCondition in(const std::vector<std::string>& vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        std::ostringstream out;
        out << "(" << name << " in (";
        size_t total = vals.size();
        for (size_t i = 0; i < total - 1; i++) {
            out << "\"" << vals[i] << "\",";
        }
        out << "\"" << vals[total - 1] << "\"))";
        return DBCondition(out.str());
    }

    template <>
    DBCondition in(const std::vector<const char*>& vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        std::ostringstream out;
        out << "(" << name << " in (";
        size_t total = vals.size();
        for (size_t i = 0; i < total - 1; i++) {
            out << "\"" << vals[i] << "\",";
        }
        out << "\"" << vals[total - 1] << "\"))";
        return DBCondition(out.str());
    }

    template <typename T>
    DBCondition not_in(const std::vector<T>& vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        return DBCondition(fmt::format("({} not in ({}))", name, fmt::join(vals, ",")));
    }

    template <>
    DBCondition not_in(const std::vector<std::string>& vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        std::ostringstream out;
        out << "(" << name << " not in (";
        size_t total = vals.size();
        for (size_t i = 0; i < total - 1; i++) {
            out << "\"" << vals[i] << "\",";
        }
        out << "\"" << vals[total - 1] << "\"))";
        return DBCondition(out.str());
    }

    template <>
    DBCondition not_in(const std::vector<const char*>& vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        std::ostringstream out;
        out << "(" << name << " not in (";
        size_t total = vals.size();
        for (size_t i = 0; i < total - 1; i++) {
            out << "\"" << vals[i] << "\",";
        }
        out << "\"" << vals[total - 1] << "\"))";
        return DBCondition(out.str());
    }

    DBCondition like(const std::string& pattern) {
        return DBCondition(fmt::format(R"(({} like "{}"))", name, pattern));
    }

    DBCondition like(const char* pattern) {
        return DBCondition(fmt::format(R"(({} like "{}"))", name, pattern));
    }

    std::string name;
};

inline std::ostream& operator<<(std::ostream& out, const DBCondition& d) {
    out << d.str();
    return out;
}

template <typename T>
inline DBCondition operator==(const Field& field, T val) {
    return DBCondition(fmt::format("({}={})", field.name, val));
}

template <typename T>
inline DBCondition operator!=(const Field& field, T val) {
    return DBCondition(fmt::format("({}<>{})", field.name, val));
}

template <typename T>
inline DBCondition operator>(const Field& field, T val) {
    return DBCondition(fmt::format("({}>{})", field.name, val));
}

template <typename T>
inline DBCondition operator>=(const Field& field, T val) {
    return DBCondition(fmt::format("({}>={})", field.name, val));
}

template <typename T>
inline DBCondition operator<(const Field& field, T val) {
    return DBCondition(fmt::format("({}<{})", field.name, val));
}

template <typename T>
inline DBCondition operator<=(const Field& field, T val) {
    return DBCondition(fmt::format("({}<={})", field.name, val));
}

template <>
inline DBCondition operator==(const Field& field, const std::string& val) {
    return DBCondition(fmt::format(R"(({}="{}"))", field.name, val));
}

template <>
inline DBCondition operator!=(const Field& field, const std::string& val) {
    return DBCondition(fmt::format(R"(({}<>"{}"))", field.name, val));
}

template <>
inline DBCondition operator>(const Field& field, const std::string& val) {
    return DBCondition(fmt::format(R"(({}>"{}"))", field.name, val));
}

template <>
inline DBCondition operator<(const Field& field, const std::string& val) {
    return DBCondition(fmt::format(R"(({}<"{}"))", field.name, val));
}

template <>
inline DBCondition operator>=(const Field& field, const std::string& val) {
    return DBCondition(fmt::format(R"(({}>="{}"))", field.name, val));
}

template <>
inline DBCondition operator<=(const Field& field, const std::string& val) {
    return DBCondition(fmt::format(R"(({}<="{}"))", field.name, val));
}

template <>
inline DBCondition operator==(const Field& field, const char* val) {
    return DBCondition(fmt::format(R"(({}="{}"))", field.name, val));
}

template <>
inline DBCondition operator!=(const Field& field, const char* val) {
    return DBCondition(fmt::format(R"(({}<>"{}"))", field.name, val));
}

template <>
inline DBCondition operator>(const Field& field, const char* val) {
    return DBCondition(fmt::format(R"(({}>"{}"))", field.name, val));
}

template <>
inline DBCondition operator<(const Field& field, const char* val) {
    return DBCondition(fmt::format(R"(({}<"{}"))", field.name, val));
}

template <>
inline DBCondition operator>=(const Field& field, const char* val) {
    return DBCondition(fmt::format(R"(({}>="{}"))", field.name, val));
}

template <>
inline DBCondition operator<=(const Field& field, const char* val) {
    return DBCondition(fmt::format(R"(({}<="{}"))", field.name, val));
}

}  // namespace hku