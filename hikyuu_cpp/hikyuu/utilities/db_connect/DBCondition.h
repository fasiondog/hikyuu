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
#include <utility>
#include <variant>
#include <vector>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/datetime/Datetime.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * Centralized escaping point for SQL string values: safely wraps any value into
 * a double-quoted string literal.
 *
 * Embedded double quotes are escaped by doubling (SQL standard), preventing
 * values from closing the literal early and breaking out into SQL injection.
 * Note: MySQL by default also recognizes backslash escape sequences, which is
 * inconsistent with SQLite; backslashes are not handled here. Also, with
 * MySQL ANSI_QUOTES enabled, "..." is parsed as an identifier (doubling still
 * keeps the token intact, no injection, but the semantics change).
 *
 * It is now the compatibility renderer used by DBCondition::str() and by the
 * plain string conditions; the parameterized path (sql()/params()) keeps values
 * out of the SQL text altogether.
 */
inline std::string sqlStringLiteral(const std::string &val) {
    std::string escaped;
    escaped.reserve(val.size() + 2);
    escaped += '"';
    for (char c : val) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += '"';
    return escaped;
}

/**
 * Centralized quoting point for SQL identifiers: safely wraps a table or column
 * name into MySQL-style backticks, which SQLite accepts as well.
 *
 * Embedded backticks are escaped by doubling. Identifiers cannot be bound as
 * parameters, so quoting is the only way to keep them out of injection.
 * Idempotent: an input already carrying a valid backtick-quoting is returned
 * unchanged, so callers may pass either a bare or a pre-quoted name; anything
 * that does not parse as a valid quoting is (re-)escaped as a bare name.
 * Empty names and names containing the null character are rejected: a null
 * byte terminates the statement early in some parsers.
 */
inline std::string sqlIdentifier(const std::string &name) {
    HKU_CHECK(!name.empty(), "Invalid table name: it is empty!");
    HKU_CHECK(name.find('\0') == std::string::npos,
              "Invalid table name: it must not contain the null character!");

    if (name.size() >= 2 && name.front() == '`' && name.back() == '`') {
        size_t i = 1;
        const size_t end = name.size() - 1;
        while (i < end) {
            if (name[i] == '`') {
                if (i + 1 < end && name[i + 1] == '`') {
                    i += 2;
                    continue;
                }
                break;
            }
            ++i;
        }
        if (i == end) {
            return name;
        }
    }

    std::string quoted;
    quoted.reserve(name.size() + 2);
    quoted += '`';
    for (char c : name) {
        if (c == '`') {
            quoted += "``";
        } else {
            quoted += c;
        }
    }
    quoted += '`';
    return quoted;
}

/**
 * Validate an identifier embedded in a condition text (a column name given to Field or orderBy).
 *
 * Identifiers cannot be bound as parameters, and the condition text carries no quoting for them,
 * so the name is checked instead: the rejected characters and sequences are exactly the ones that
 * would either break out into SQL or be mistaken for a placeholder / literal / comment marker by
 * the scanner used to render and renumber conditions. Names that pass produce the very same SQL
 * text as before, which keeps the rendered condition byte-for-byte compatible.
 */
inline void validateSqlName(const std::string &name) {
    HKU_CHECK(!name.empty(), "Invalid column name: it is empty!");
    HKU_CHECK(name.find('\0') == std::string::npos,
              "Invalid column name: it must not contain the null character!");
    for (char c : name) {
        HKU_CHECK(c != '"' && c != '\'' && c != '`' && c != '?' && c != ';' && c != '#',
                  "Invalid column name: {}!", name);
    }
    HKU_CHECK(name.find("--") == std::string::npos && name.find("/*") == std::string::npos &&
                name.find("*/") == std::string::npos,
              "Invalid column name: {}!", name);
}

/**
 * A value bound to a ? placeholder of a parameterized condition.
 *
 * Only string values are parameterized: they are the injection surface and the one type the
 * static escaping cannot cover reliably. Numbers and dates keep the inline form of the existing
 * behaviour, so they never appear here.
 */
typedef std::variant<std::nullptr_t, int64_t, double, std::string, Datetime> BoundValue;

/** @ingroup DBConnect */
typedef std::vector<BoundValue> BoundValues;

/**
 * The maximum number of placeholders in one statement.
 *
 * SQLite allows SQLITE_MAX_VARIABLE_NUMBER parameters (32766 by default since 3.32, 999 in older
 * builds) and MySQL prepared statements at most 65535. Exceeding the limit is reported early
 * instead of producing a statement the driver is guaranteed to reject. A build linking an old
 * SQLite with a 999 limit has to lower this constant accordingly.
 */
constexpr size_t MAX_SQL_BIND_PARAMS = 32766;

/** The parts of a plain where clause: the filter, the trailing order-by and the row limit */
struct WhereParts {
    std::string where;    //<! the filter part, may be empty
    std::string orderBy;  //<! the trailing "order by" clause with its leading space, may be empty
    int limit = -1;       //<! the trailing row limit, -1 means none
};

/**
 * Split a plain (hand-written) where clause into its parts.
 *
 * The trailing "order by" / "limit" clauses are located by scanning the text outside string
 * literals, quoted identifiers and comments, and only at a word boundary, so that a column named
 * like order_date or amount_limit is not mistaken for the start of a clause. An empty or absent
 * clause is reported as such. This is only for the plain string path: a DBCondition carries its
 * parts already split, so no parsing is needed there.
 */
HKU_UTILS_API WhereParts splitWhereParts(const std::string &where);

/**
 * Validate a parameterized condition and rewrite its numbered placeholders to the anonymous form.
 *
 * A condition fragment refers to its values as ?1..?k, which keeps the value order checkable while
 * the fragment is combined and embedded into a statement template. Drivers are fed the anonymous
 * ? instead (MySQL supports nothing else, and binding stays a plain left-to-right sequence), so
 * the numbers are dropped here. Checks performed:
 * - the placeholders are exactly ?1..?k, each one time, and k == params.size();
 * - numbered and anonymous placeholders are never mixed in one statement (the drivers number them
 *   differently, which would silently shift every value after the mix);
 * - the total number of placeholders stays within MAX_SQL_BIND_PARAMS.
 *
 * A statement without numbered placeholders is returned unchanged, so the plain path costs nothing
 * extra. An hku::exception is thrown when a check fails.
 */
HKU_UTILS_API std::pair<std::string, BoundValues> renumberPlaceholders(const std::string &sql,
                                                                       BoundValues params);

struct ASC {
    explicit ASC(const char *name) : name(name) {}
    explicit ASC(const std::string &name) : name(name) {}
    std::string name;
};

struct DESC {
    explicit DESC(const char *name) : name(name) {}
    explicit DESC(const std::string &name) : name(name) {}
    std::string name;
};

struct LIMIT {
    explicit LIMIT(int limit) : limit(limit) {}
    int limit = 1;
};

/**
 * Query condition of a table model query.
 *
 * A condition carries two views of itself: the parameterized one (sql() + params(), values kept
 * outside of the SQL text as ?1..?k placeholders) and the rendered one (str(), values escaped as
 * literals). str() is rendered eagerly whenever the condition changes and is byte-for-byte the
 * same text as before parameterization, so existing callers that splice it into their own SQL keep
 * working unchanged; the library itself uses the parameterized view.
 *
 * The order-by and limit clauses are kept apart from the filter, both because they cannot be
 * parameterized and because the paged query needs them in another position of the statement.
 */
class HKU_UTILS_API DBCondition {
public:
    DBCondition() = default;
    DBCondition(const DBCondition &) = default;
    DBCondition(DBCondition &&rv) = default;

    /** Build a condition from a plain (hand-written) SQL text, without parameters */
    explicit DBCondition(const char *cond) : m_sql(cond), m_condition(cond) {}

    /** Build a condition from a plain (hand-written) SQL text, without parameters */
    explicit DBCondition(const std::string &cond) : m_sql(cond), m_condition(cond) {}

    DBCondition &operator=(const DBCondition &) = default;
    DBCondition &operator=(DBCondition &&rv) = default;

    /**
     * Build a condition from a parameterized fragment
     * @param sql the fragment text, referring to its values as ?1..?k
     * @param params the values of the placeholders, in placeholder order
     */
    static DBCondition fromFragment(std::string sql, BoundValues params);

    DBCondition &operator&(const DBCondition &other);
    DBCondition &operator|(const DBCondition &other);

    enum ORDERBY { ORDER_ASC, ORDER_DESC };

    /** Append an order-by clause; names are validated, identifiers cannot be bound */
    void orderBy(const std::string &field, ORDERBY order);

    DBCondition &operator+(const ASC &asc);
    DBCondition &operator+(const DESC &desc);
    DBCondition &operator+(const LIMIT &limit);

    /** The condition rendered with escaped literals, as the plain string API has always returned */
    const std::string &str() const {
        return m_condition;
    }

    /** The filter part with numbered ? placeholders, empty when the condition is empty */
    const std::string &sql() const {
        return m_sql;
    }

    /** The values of the placeholders of sql(), in placeholder order */
    const BoundValues &params() const {
        return m_params;
    }

    /** Whether the condition carries any bindable value */
    bool hasParams() const {
        return !m_params.empty();
    }

    /** The trailing order-by clause, including its leading space, empty when there is none */
    const std::string &getOrderBy() const {
        return m_orderBy;
    }

    /** The row limit set by LIMIT, -1 when there is none */
    int getLimit() const {
        return m_limit;
    }

private:
    /** Rebuild the rendered view from the parts */
    void _render();

    /** Merge another condition into this one with the given operator, keeping the placeholder
        numbering of both sides consistent and the order-by / limit clauses at the end */
    DBCondition &_combine(const DBCondition &other, const char *op);

    std::string m_sql;        //<! the filter part, with ?1..?k placeholders
    BoundValues m_params;     //<! the values of the placeholders
    std::string m_orderBy;    //<! the trailing order-by clause, never parameterized
    int m_limit = -1;         //<! the trailing row limit, -1 means none
    std::string m_condition;  //<! the rendered view, kept in sync by _render()
};

/**
 * Split a condition into the where clause it contributes to a statement and its bindable values.
 *
 * The clause keeps its ?1..?k placeholders and the order-by / limit clauses stay at its end, where
 * the statement templates expect them. Handing the pair to renumberPlaceholders together with the
 * statement built around it is all that a driver needs. A condition without values yields the very
 * same text as str(), so the plain path is unchanged.
 */
inline std::pair<std::string, BoundValues> conditionParts(const DBCondition &cond) {
    std::string where = cond.sql() + cond.getOrderBy();
    if (cond.getLimit() >= 0) {
        where += fmt::format(" limit {}", cond.getLimit());
    }
    return {std::move(where), cond.params()};
}

struct Field {
    explicit Field(const char *name) : name(name) {
        validateSqlName(name);
    }

    explicit Field(const std::string &name) : name(name) {
        validateSqlName(name);
    }

    // in and not_in do not support strings; the SQL operation in ("stra", "strb") is generally not
    // used
    template <typename T>
    DBCondition in(const std::vector<T> &vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        return DBCondition(fmt::format("({} in ({}))", name, fmt::join(vals, ",")));
    }

    template <typename T>
    DBCondition not_in(const std::vector<T> &vals) {
        HKU_CHECK(!vals.empty(), "input vals can't be empty!");
        return DBCondition(fmt::format("({} not in ({}))", name, fmt::join(vals, ",")));
    }

    DBCondition like(const std::string &pattern) {
        return DBCondition::fromFragment(fmt::format("({} like ?1)", name), {pattern});
    }

    DBCondition like(const char *pattern) {
        return DBCondition::fromFragment(fmt::format("({} like ?1)", name), {std::string(pattern)});
    }

    std::string name;
};

// Under linux the template specialization of a class member function must be implemented outside
// the class
// Otherwise the compilation reports: explicit specialization in non-namespace scope
template <>
inline DBCondition Field::in<std::string>(const std::vector<std::string> &vals) {
    HKU_CHECK(!vals.empty(), "input vals can't be empty!");
    HKU_CHECK(vals.size() <= MAX_SQL_BIND_PARAMS,
              "The in condition has {} values, over the limit of {} placeholders!", vals.size(),
              MAX_SQL_BIND_PARAMS);
    std::vector<std::string> slots;
    slots.reserve(vals.size());
    BoundValues params;
    params.reserve(vals.size());
    for (size_t i = 0, len = vals.size(); i < len; ++i) {
        slots.push_back(fmt::format("?{}", i + 1));
        params.push_back(vals[i]);
    }
    return DBCondition::fromFragment(fmt::format("({} in ({}))", name, fmt::join(slots, ",")),
                                     std::move(params));
}

template <>
inline DBCondition Field::not_in<std::string>(const std::vector<std::string> &vals) {
    HKU_CHECK(!vals.empty(), "input vals can't be empty!");
    HKU_CHECK(vals.size() <= MAX_SQL_BIND_PARAMS,
              "The not in condition has {} values, over the limit of {} placeholders!", vals.size(),
              MAX_SQL_BIND_PARAMS);
    std::vector<std::string> slots;
    slots.reserve(vals.size());
    BoundValues params;
    params.reserve(vals.size());
    for (size_t i = 0, len = vals.size(); i < len; ++i) {
        slots.push_back(fmt::format("?{}", i + 1));
        params.push_back(vals[i]);
    }
    return DBCondition::fromFragment(fmt::format("({} not in ({}))", name, fmt::join(slots, ",")),
                                     std::move(params));
}

// A pointer list carries text values as well: it must not fall back to the generic template, which
// splices them into the statement unquoted
template <>
inline DBCondition Field::in<const char *>(const std::vector<const char *> &vals) {
    HKU_CHECK(!vals.empty(), "input vals can't be empty!");
    std::vector<std::string> values;
    values.reserve(vals.size());
    for (const auto val : vals) {
        values.emplace_back(val);
    }
    return in(values);
}

template <>
inline DBCondition Field::not_in<const char *>(const std::vector<const char *> &vals) {
    HKU_CHECK(!vals.empty(), "input vals can't be empty!");
    std::vector<std::string> values;
    values.reserve(vals.size());
    for (const auto val : vals) {
        values.emplace_back(val);
    }
    return not_in(values);
}

inline std::ostream &operator<<(std::ostream &out, const DBCondition &d) {
    out << d.str();
    return out;
}

template <typename T>
inline DBCondition operator==(const Field &field, T val) {
    std::ostringstream out;
    out << "(" << field.name << "=" << val << ")";
    return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator!=(const Field &field, T val) {
    std::ostringstream out;
    out << "(" << field.name << "<>" << val << ")";
    return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator>(const Field &field, T val) {
    std::ostringstream out;
    out << "(" << field.name << ">" << val << ")";
    return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator>=(const Field &field, T val) {
    std::ostringstream out;
    out << "(" << field.name << ">=" << val << ")";
    return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator<(const Field &field, T val) {
    std::ostringstream out;
    out << "(" << field.name << "<" << val << ")";
    return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator<=(const Field &field, T val) {
    std::ostringstream out;
    out << "(" << field.name << "<=" << val << ")";
    return DBCondition(out.str());
}

inline DBCondition operator==(const Field &field, const std::string &val) {
    return DBCondition::fromFragment(fmt::format("({}=?1)", field.name), {val});
}

inline DBCondition operator!=(const Field &field, const std::string &val) {
    return DBCondition::fromFragment(fmt::format("({}<>{})", field.name, "?1"), {val});
}

inline DBCondition operator>(const Field &field, const std::string &val) {
    return DBCondition::fromFragment(fmt::format("({}>?1)", field.name), {val});
}

inline DBCondition operator<(const Field &field, const std::string &val) {
    return DBCondition::fromFragment(fmt::format("({}<?1)", field.name), {val});
}

inline DBCondition operator>=(const Field &field, const std::string &val) {
    return DBCondition::fromFragment(fmt::format("({}>=?1)", field.name), {val});
}

inline DBCondition operator<=(const Field &field, const std::string &val) {
    return DBCondition::fromFragment(fmt::format("({}<=?1)", field.name), {val});
}

// String literals and char pointers are bound as values as well: without these overloads the
// generic template above would be selected and stream the pointer address instead of the text
inline DBCondition operator==(const Field &field, const char *val) {
    return DBCondition::fromFragment(fmt::format("({}=?1)", field.name), {std::string(val)});
}

inline DBCondition operator!=(const Field &field, const char *val) {
    return DBCondition::fromFragment(fmt::format("({}<>{})", field.name, "?1"), {std::string(val)});
}

inline DBCondition operator>(const Field &field, const char *val) {
    return DBCondition::fromFragment(fmt::format("({}>?1)", field.name), {std::string(val)});
}

inline DBCondition operator<(const Field &field, const char *val) {
    return DBCondition::fromFragment(fmt::format("({}<?1)", field.name), {std::string(val)});
}

inline DBCondition operator>=(const Field &field, const char *val) {
    return DBCondition::fromFragment(fmt::format("({}>=?1)", field.name), {std::string(val)});
}

inline DBCondition operator<=(const Field &field, const char *val) {
    return DBCondition::fromFragment(fmt::format("({}<=?1)", field.name), {std::string(val)});
}

}  // namespace hku
