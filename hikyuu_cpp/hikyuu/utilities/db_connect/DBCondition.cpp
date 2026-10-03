/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-20
 *     Author: fasiondog
 */

#include <algorithm>
#include <cctype>
#include <cstdio>
#include "DBCondition.h"

namespace hku {
namespace {

/** A placeholder occurrence found by the scanner */
struct Placeholder {
    size_t pos;  //<! index of the ? character
    size_t len;  //<! token length, 1 for the anonymous form
    long num;    //<! placeholder number, 0 for the anonymous form
};

inline bool _isIdentChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

inline bool _isQuoteChar(char c) {
    return c == '\'' || c == '"' || c == '`';
}

/**
 * Skip an opaque span starting at i (a string literal or a quoted identifier) and return the index
 * just after its closing character, or the string size when it is never closed. A doubled quote is
 * the escape form of the quoted content and does not close the span. A backslash inside a string
 * literal is not an escape here (SQLite semantics); a MySQL dialect literal written with \' keeps
 * scanning until its real closing quote, which only shifts the opaque span and cannot misbind.
 */
size_t skipOpaque(const std::string &sql, size_t i) {
    const size_t len = sql.size();
    const char quote = sql[i];
    size_t j = i + 1;
    while (j < len) {
        if (sql[j] == quote) {
            if (j + 1 < len && sql[j + 1] == quote) {
                j += 2;
                continue;
            }
            return j + 1;
        }
        ++j;
    }
    return len;
}

/** Skip a "--" or "#" line comment and return the index of its line terminator (or the end) */
size_t skipLineComment(const std::string &sql, size_t i) {
    size_t j = sql.find('\n', i);
    return j == std::string::npos ? sql.size() : j;
}

/** Skip a block comment and return the index just after its terminator (or the end) */
size_t skipBlockComment(const std::string &sql, size_t i) {
    size_t j = sql.find("*/", i + 2);
    return j == std::string::npos ? sql.size() : j + 2;
}

/** Render one bound value back into SQL text, the way the inline path always did */
std::string renderValue(const BoundValue &value) {
    return std::visit(
      [](const auto &v) -> std::string {
          using U = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<U, std::nullptr_t>) {
              return "NULL";
          } else if constexpr (std::is_same_v<U, std::string>) {
              return sqlStringLiteral(v);
          } else {
              std::ostringstream out;
              out << v;
              return out.str();
          }
      },
      value);
}

/** Collect the placeholders of a statement, ignoring quoted spans and comments */
std::vector<Placeholder> scanPlaceholders(const std::string &sql) {
    std::vector<Placeholder> marks;
    const size_t len = sql.size();
    size_t i = 0;
    while (i < len) {
        const char c = sql[i];
        if (_isQuoteChar(c)) {
            i = skipOpaque(sql, i);
            continue;
        }
        if (c == '-' && i + 1 < len && sql[i + 1] == '-') {
            i = skipLineComment(sql, i);
            continue;
        }
        if (c == '#') {
            i = skipLineComment(sql, i);
            continue;
        }
        if (c == '/' && i + 1 < len && sql[i + 1] == '*') {
            i = skipBlockComment(sql, i);
            continue;
        }
        if (c == '?') {
            Placeholder mark{i, 1, 0};
            size_t j = i + 1;
            long num = 0;
            bool numbered = false;
            while (j < len && std::isdigit(static_cast<unsigned char>(sql[j])) != 0) {
                num = num * 10 + (sql[j] - '0');
                numbered = true;
                ++j;
            }
            if (numbered) {
                mark.len = j - i;
                mark.num = num;
            }
            marks.push_back(mark);
            i = j;
            continue;
        }
        ++i;
    }
    return marks;
}

/** Split the marks of a statement into numbered and anonymous ones */
void countMarks(const std::vector<Placeholder> &marks, size_t &numbered, size_t &bare) {
    numbered = bare = 0;
    for (const auto &m : marks) {
        m.num == 0 ? ++bare : ++numbered;
    }
}

/** A short leading slice of the statement, for error messages */
std::string brief(const std::string &sql) {
    return sql.size() > 200 ? sql.substr(0, 200) + "..." : sql;
}

/** Whether any of the marks uses the numbered form */
bool hasNumbered(const std::vector<Placeholder> &marks) {
    for (const auto &m : marks) {
        if (m.num != 0) {
            return true;
        }
    }
    return false;
}

/**
 * Check that the numbered marks are exactly ?1..?k, each one time, matching param_count values and
 * staying within the driver limit.
 */
void _checkNumbered(const std::vector<Placeholder> &marks, size_t param_count, size_t numbered,
                    const std::string &sql) {
    HKU_CHECK(numbered == param_count, "The condition has {} placeholders but {} values! sql: {}",
              numbered, param_count, brief(sql));
    HKU_CHECK(numbered <= MAX_SQL_BIND_PARAMS,
              "The condition has {} placeholders, over the limit of {}! sql: {}", numbered,
              MAX_SQL_BIND_PARAMS, brief(sql));

    std::vector<bool> used(numbered + 1, false);
    for (const auto &m : marks) {
        if (m.num == 0) {
            continue;
        }
        HKU_CHECK(static_cast<size_t>(m.num) <= numbered && !used[m.num],
                  "The placeholder ?{} is out of range or repeated! sql: {}", m.num, brief(sql));
        used[m.num] = true;
    }
    for (size_t num = 1; num <= numbered; ++num) {
        HKU_CHECK(used[num], "The placeholder ?{} is missing! sql: {}", num, brief(sql));
    }
}

/**
 * Collect the positions of every standalone occurrence of the keyword inside a text, ignoring the
 * quoted spans and comments, and matching it case-insensitively at a word boundary
 */
std::vector<size_t> findKeywords(const std::string &text, const std::string &keyword) {
    std::vector<size_t> found;
    const size_t len = text.size();
    const size_t klen = keyword.size();
    size_t i = 0;
    while (i < len) {
        const char c = text[i];
        if (_isQuoteChar(c)) {
            i = skipOpaque(text, i);
            continue;
        }
        if (c == '-' && i + 1 < len && text[i + 1] == '-') {
            i = skipLineComment(text, i);
            continue;
        }
        if (c == '#') {
            i = skipLineComment(text, i);
            continue;
        }
        if (c == '/' && i + 1 < len && text[i + 1] == '*') {
            i = skipBlockComment(text, i);
            continue;
        }
        if (_isIdentChar(c) && i + klen <= len) {
            bool hit = true;
            for (size_t k = 0; k < klen; ++k) {
                if (std::toupper(static_cast<unsigned char>(text[i + k])) !=
                    std::toupper(static_cast<unsigned char>(keyword[k]))) {
                    hit = false;
                    break;
                }
            }
            const bool left = (i == 0) || !_isIdentChar(text[i - 1]);
            const bool right = (i + klen == len) || !_isIdentChar(text[i + klen]);
            if (hit && left && right) {
                found.push_back(i);
                i += klen;
                continue;
            }
        }
        ++i;
    }
    return found;
}

/** Whether the next word after the keyword at pos is the given one, skipping plain spaces */
bool followedByWord(const std::string &text, size_t pos, size_t klen, const std::string &next) {
    const size_t len = text.size();
    size_t i = pos + klen;
    while (i < len && std::isspace(static_cast<unsigned char>(text[i])) != 0) {
        ++i;
    }
    if (i + next.size() > len) {
        return false;
    }
    for (size_t k = 0; k < next.size(); ++k) {
        if (std::toupper(static_cast<unsigned char>(text[i + k])) !=
            std::toupper(static_cast<unsigned char>(next[k]))) {
            return false;
        }
    }
    const size_t j = i + next.size();
    return j == len || !_isIdentChar(text[j]);
}

/** Read the row limit written after the keyword at pos, -1 when it is not a plain integer */
int parseLimitAt(const std::string &text, size_t pos) {
    const size_t len = text.size();
    size_t i = pos + 5;  // the length of the "limit" keyword
    while (i < len && std::isspace(static_cast<unsigned char>(text[i])) != 0) {
        ++i;
    }
    if (i >= len || std::isdigit(static_cast<unsigned char>(text[i])) == 0) {
        return -1;
    }
    long long value = 0;
    while (i < len && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
        value = value * 10 + (text[i] - '0');
        if (value > 0x7ffffffeLL) {
            return -1;
        }
        ++i;
    }
    // the limit has to end the clause: anything right after the number (a second number of the
    // MySQL "limit n, m" form, a semicolon, ...) is not a plain limit and is left in the text
    while (i < len && std::isspace(static_cast<unsigned char>(text[i])) != 0) {
        ++i;
    }
    if (i < len) {
        return -1;
    }
    return static_cast<int>(value);
}

/**
 * Move the numbered placeholders of a fragment up by offset, so that a fragment keeps referring to
 * the same values after it has been appended to another one
 */
std::string shiftPlaceholderNumbers(const std::string &sql, size_t offset) {
    if (sql.empty()) {
        return sql;
    }
    if (offset == 0) {
        return sql;
    }

    const std::vector<Placeholder> marks = scanPlaceholders(sql);
    std::string out;
    out.reserve(sql.size() + marks.size() * 2);
    size_t last = 0;
    for (const auto &m : marks) {
        out.append(sql, last, m.pos - last);
        out += m.num == 0 ? std::string("?") : fmt::format("?{}", m.num + offset);
        last = m.pos + m.len;
    }
    out.append(sql, last, std::string::npos);
    return out;
}

}  // namespace

DBCondition DBCondition::fromFragment(std::string sql, BoundValues params) {
    // a fragment always has to keep its placeholders and its values in step: an unpaired ?N would
    // otherwise be sent to the driver as text or a value would be dropped silently
    if (params.empty()) {
        HKU_CHECK(!hasNumbered(scanPlaceholders(sql)),
                  "The parameterized condition has placeholders but no values! sql: {}",
                  brief(sql));
    }

    DBCondition cond;
    cond.m_sql = std::move(sql);
    cond.m_params = std::move(params);
    cond._render();
    return cond;
}

void DBCondition::_render() {
    // the plain path keeps the text as it was written: a hand-written condition is never re-parsed
    if (m_params.empty()) {
        m_condition =
          m_sql + m_orderBy + (m_limit < 0 ? std::string() : fmt::format(" limit {}", m_limit));
        return;
    }

    std::vector<Placeholder> marks = scanPlaceholders(m_sql);
    size_t numbered = 0, bare = 0;
    countMarks(marks, numbered, bare);
    HKU_CHECK(bare == 0, "Mixing anonymous and numbered placeholders is not supported! sql: {}",
              brief(m_sql));
    _checkNumbered(marks, m_params.size(), numbered, m_sql);

    std::string out;
    out.reserve(m_sql.size() + m_params.size() * 8);
    size_t last = 0;
    for (const auto &m : marks) {
        out.append(m_sql, last, m.pos - last);
        out += renderValue(m_params[m.num - 1]);
        last = m.pos + m.len;
    }
    out.append(m_sql, last, std::string::npos);

    out += m_orderBy;
    if (m_limit >= 0) {
        out += fmt::format(" limit {}", m_limit);
    }
    m_condition = std::move(out);
}

DBCondition &DBCondition::_combine(const DBCondition &other, const char *op) {
    if (this == &other) {
        return *this;
    }

    const bool self_empty = m_sql.empty() && m_params.empty() && m_orderBy.empty() && m_limit < 0;
    const bool other_empty =
      other.m_sql.empty() && other.m_params.empty() && other.m_orderBy.empty() && other.m_limit < 0;

    // an empty other used to be rendered as "(A and )", which no driver accepts; merging it is a
    // no-op instead
    HKU_IF_RETURN(other_empty, *this);
    HKU_IF_RETURN(self_empty, (*this = other));

    // the values of both sides keep their relative order, so the placeholders of the right side
    // continue the numbering of the left side
    const size_t offset = m_params.size();
    const std::string right = shiftPlaceholderNumbers(other.m_sql, offset);

    // a side without a filter of its own (an order-by or a limit only) contributes nothing to the
    // combined filter, instead of leaving a dangling "(A and )"
    if (!m_sql.empty() && !right.empty()) {
        m_sql = fmt::format("({} {} {})", m_sql, op, right);
    } else if (!right.empty()) {
        m_sql = right;
    }

    m_params.insert(m_params.end(), other.m_params.begin(), other.m_params.end());
    m_orderBy += other.m_orderBy;
    if (other.m_limit >= 0) {
        m_limit = other.m_limit;  // only one trailing limit clause fits into a statement
    }
    _render();
    return *this;
}

DBCondition &DBCondition::operator&(const DBCondition &other) {
    return _combine(other, "and");
}

DBCondition &DBCondition::operator|(const DBCondition &other) {
    return _combine(other, "or");
}

void DBCondition::orderBy(const std::string &field, ORDERBY order) {
    validateSqlName(field);
    m_orderBy += order == ORDER_ASC ? fmt::format(" order by {} ASC", field)
                                    : fmt::format(" order by {} DESC", field);
    _render();
}

DBCondition &DBCondition::operator+(const ASC &asc) {
    orderBy(asc.name, ORDER_ASC);
    return *this;
}

DBCondition &DBCondition::operator+(const DESC &desc) {
    orderBy(desc.name, ORDER_DESC);
    return *this;
}

DBCondition &DBCondition::operator+(const LIMIT &limit) {
    m_limit = limit.limit;
    _render();
    return *this;
}

std::pair<std::string, BoundValues> renumberPlaceholders(const std::string &sql,
                                                         BoundValues params) {
    std::vector<Placeholder> marks = scanPlaceholders(sql);
    size_t numbered = 0, bare = 0;
    countMarks(marks, numbered, bare);

    // nothing to rewrite: the plain path, or a statement written entirely with the anonymous form
    if (numbered == 0) {
        HKU_CHECK(bare == 0 ? params.empty() : bare == params.size(),
                  "The statement has {} placeholders but {} values! sql: {}", bare, params.size(),
                  brief(sql));
        return {sql, std::move(params)};
    }

    HKU_CHECK(bare == 0, "Mixing anonymous and numbered placeholders is not supported! sql: {}",
              brief(sql));
    _checkNumbered(marks, params.size(), numbered, sql);

    // the values are ordered by appearance so that binding stays a plain left-to-right sequence
    std::string out;
    out.reserve(sql.size());
    BoundValues ordered;
    ordered.reserve(numbered);
    size_t last = 0;
    for (const auto &m : marks) {
        out.append(sql, last, m.pos - last);
        out += '?';
        last = m.pos + m.len;
        ordered.push_back(params[m.num - 1]);
    }
    out.append(sql, last, std::string::npos);

    return {std::move(out), std::move(ordered)};
}

WhereParts splitWhereParts(const std::string &where) {
    WhereParts parts;

    // an order clause is only recognized as the pair "order by", so that a column simply named
    // order does not open one
    size_t order_pos = std::string::npos;
    for (size_t pos : findKeywords(where, "order")) {
        if (followedByWord(where, pos, 5, "by")) {
            order_pos = pos;
        }
    }

    size_t limit_pos = std::string::npos;
    int limit = -1;
    for (size_t pos : findKeywords(where, "limit")) {
        const int value = parseLimitAt(where, pos);
        if (value >= 0) {
            limit_pos = pos;
            limit = value;
        }
    }

    if (order_pos == std::string::npos && limit_pos == std::string::npos) {
        parts.where = where;
        return parts;
    }

    const size_t cut = std::min(order_pos, limit_pos);
    parts.where = where.substr(0, cut);

    if (order_pos != std::string::npos && limit_pos != std::string::npos && limit_pos < order_pos) {
        // "limit 5 order by x" is not a valid statement shape, keeping the text as written leaves
        // the driver to report it instead of rewriting the caller's SQL
        parts.orderBy = where.substr(cut);
        return parts;
    }

    parts.limit = limit;
    if (order_pos != std::string::npos) {
        parts.orderBy = limit_pos != std::string::npos && order_pos < limit_pos
                          ? where.substr(order_pos, limit_pos - order_pos)
                          : where.substr(order_pos);
    }
    return parts;
}

}  // namespace hku
