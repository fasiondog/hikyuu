/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-01-10
 *      Author: fasiondog
 */

#pragma once

#include <algorithm>
#include <iterator>
#include "hikyuu/utilities/arithmetic.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/osdef.h"
#include "DBConnectBase.h"

namespace hku {

template <class TableT, size_t page_size>
class SQLResultSetIterator;

/**
 * SQL query result set
 * @tparam TableT data structure
 * @tparam page_size the number of the data contained in every page
 * @ingroup DBConnect
 *
 * @note Not thread safe: a result set is designed to be used in a single thread (or a single
 *       coroutine). Its internal page cache is filled on demand without any synchronization,
 *       so sharing one instance across threads would race; give every thread its own instance
 *       instead (the underlying connection may still be shared, the drivers serialize it)
 *
 * @note Precondition: TableT must be bound through the TABLE_BIND macros, which make the whole
 *       ORM layer rely on an integer primary key column named "id" (getSelectSQL selects it as
 *       column 0, update/remove filter on it, valid() and load() read it). Paged query depends on
 *       it too: the page window is built as `id IN (SELECT id ... ORDER BY id ...)` and the outer
 *       select re-applies the same order. A table without such an "id" column is not supported by
 *       any query path (paged or not), not just this one
 */
template <class TableT, size_t page_size = 100>
class SQLResultSet {
    friend class SQLResultSetIterator<TableT, page_size>;

public:
#if CPP_STANDARD >= CPP_STANDARD_17
    static constexpr int PSIZE = page_size;
#else
    static const int PSIZE = page_size;
#endif

    SQLResultSet() = default;

    /**
     * Build a new paged query result instance
     * @param connect
     * @param sql
     */
    SQLResultSet(const DBConnectPtr& connect, const std::string& sql)
    : m_connect(connect),
      m_sql_template("id IN (SELECT id FROM {} WHERE {} {} LIMIT {} OFFSET {}) {}") {
        // the plain path: the clauses can only be located in the text the caller wrote
        WhereParts parts = splitWhereParts(sql);
        _setup(parts.where, parts.orderBy, parts.limit, BoundValues{});
    }

    /**
     * Build a new paged query result instance from a condition
     * @param connect
     * @param cond the query condition, whose values stay bound to placeholders
     */
    SQLResultSet(const DBConnectPtr& connect, const DBCondition& cond)
    : m_connect(connect),
      m_sql_template("id IN (SELECT id FROM {} WHERE {} {} LIMIT {} OFFSET {}) {}") {
        // a condition already carries its parts, nothing has to be parsed out of the text
        _setup(cond.sql(), cond.getOrderBy(), cond.getLimit(), cond.params());
    }

    /** Get its database connection */
    const DBConnectPtr& getConnect() const {
        return m_connect;
    }

    using const_iterator = SQLResultSetIterator<TableT, page_size>;
    using iterator = SQLResultSetIterator<TableT, page_size>;

    const_iterator cbegin() {
        return const_iterator(this, 0);
    }

    const_iterator cend() {
        return const_iterator(this, Null<size_t>());
    }

    iterator begin() {
        return iterator(this, 0);
    }

    iterator end() {
        return iterator(this, Null<size_t>());
    }

    /**
     * @brief Get the data set size at the current moment
     * @note The data set size changes with the current database content, it is not always constant
     * @return size_t
     */
    size_t size() const {
        HKU_IF_RETURN(!m_connect, 0);
        std::string sql =
          fmt::format("select count(1) from {} where {}", TableT::getTableName(), m_where);
        size_t total = m_connect->queryNumber<size_t>(sql, 0, m_params);
        // the row limit of the condition caps the reported size, the pages are cut the same way
        return m_limit >= 0 ? std::min(total, static_cast<size_t>(m_limit)) : total;
    }

    /**
     * @brief Whether the current data set is empty
     * @return true empty
     * @return false not empty
     */
    bool empty() const {
        return size() == 0;
    }

    /**
     * @brief Get the number of the pages of the current data set
     * @note It is the number of the pages of the corresponding data set obtained at the calling
     * moment only
     * @return size_t
     */
    size_t getPageCount() {
        size_t total = size();
        size_t n = total / page_size;
        // the last page counts only when it holds rows: an exact multiple needs no extra page
        return n * page_size >= total ? n : n + 1;
    }

    /**
     * @brief Get all the data in the given page
     * @param page the given page
     * @return std::vector<TableT> all the valid data sets contained in this page
     */
    std::vector<TableT> getPage(size_t page) {
        std::vector<TableT> result;
        HKU_IF_RETURN(!m_connect || _pageLimit(page) <= 0, result);
        m_connect->batchLoadView(result, _selectSQL(page), m_params);
        return result;
    }

    TableT operator[](size_t index) {
        return get(index);
    }

    TableT at(size_t index) {
        TableT result = get(index);
        HKU_CHECK_THROW(result.valid(), std::out_of_range, "Index is over");
        return result;
    }

private:
    /** Prepare the parts of the query: the filter, the order-by of the inner and outer select */
    void _setup(const std::string& where, const std::string& orderBy, int limit,
                BoundValues params) {
        m_params = std::move(params);
        m_limit = limit;

        std::string text = where;
        trim(text);
        m_where = text.empty() ? "1=1" : text;

        if (orderBy.empty()) {
            m_orderby_inner = "ORDER BY id";
        } else {
            m_orderby_inner = fmt::format("{}, id ASC", orderBy);
        }

        // the page rows are picked by id in the inner subquery, but the outer select reads them
        // back with `id IN (...)`, whose result order is undefined without an ORDER BY of its own;
        // without it the row sequence within a page (and hence the index-to-row mapping) is not
        // guaranteed on every driver, so the outer query must re-apply the same order as the inner
        m_orderby_outer = m_orderby_inner;
    }

    /**
     * The number of rows the given page has to fetch, or 0 when the row limit of the condition is
     * already used up by the earlier pages
     */
    size_t _pageLimit(size_t page) const {
        if (m_limit < 0) {
            return page_size;
        }

        const size_t offset = page * page_size;
        const size_t left = static_cast<size_t>(m_limit);
        return offset >= left ? 0 : std::min(page_size, left - offset);
    }

    /** The full select statement of the given page, the values still bound to placeholders */
    std::string _selectSQL(size_t page) const {
        return fmt::format(
          "{} where {}", TableT::getSelectSQL(),
          fmt::format(fmt::runtime(m_sql_template), TableT::getTableName(), m_where,
                      m_orderby_inner, _pageLimit(page), page * page_size, m_orderby_outer));
    }

    TableT get(size_t index) {
        TableT result{Null<TableT>()};
        HKU_IF_RETURN(index == Null<size_t>(), result);

        size_t page = index / page_size;
        if (m_connect && page != m_current_page) {
            m_buffer.clear();
            if (_pageLimit(page) > 0) {
                m_connect->batchLoadView(m_buffer, _selectSQL(page), m_params);
            }
            m_current_page = page;
        }

        HKU_IF_RETURN(m_buffer.empty(), result);

        size_t pos = index - page * page_size;
        HKU_IF_RETURN(pos >= m_buffer.size(), result);

        result = m_buffer[index - page * page_size];
        return result;
    }

private:
    DBConnectPtr m_connect;
    std::vector<TableT> m_buffer;
    std::string m_where;
    std::string m_sql_template;
    std::string m_orderby_inner;
    std::string m_orderby_outer;
    BoundValues m_params;
    int m_limit = -1;
    size_t m_current_page = Null<size_t>();
};

template <class TableT, size_t page_size>
class SQLResultSetIterator {
public:
    using ResultSet = SQLResultSet<TableT, page_size>;

    SQLResultSetIterator() = default;
    ~SQLResultSetIterator() = default;

    explicit SQLResultSetIterator(ResultSet* result_set, size_t index)
    : m_set(result_set), m_index(index) {
        if (m_index != Null<size_t>()) {
            m_value = std::move(m_set->get(index));
            if (!m_value.valid()) {
                m_index = Null<size_t>();
            }
        }
    }

    SQLResultSetIterator(const SQLResultSetIterator& other)
    : m_set(other.m_set), m_index(other.m_index), m_value(other.m_value) {}

    SQLResultSetIterator& operator=(const SQLResultSetIterator& other) {
        if (this == &other)
            return *this;
        m_index = other.m_index;
        m_set = other.m_set;
        m_value = other.m_value;
        return *this;
    }

    const TableT& operator*() const {
        return m_value;
    }

    TableT& operator*() {
        return m_value;
    }

    const TableT* const operator->() const {
        return &m_value;
    }

    TableT* operator->() {
        return &m_value;
    }

    // Prefix increment operator
    SQLResultSetIterator& operator++() {
        HKU_CHECK_THROW(m_index != Null<size_t>(), std::logic_error,
                        "Cannot increment an end iterator.");
        m_index++;
        m_value = std::move(m_set->get(m_index));
        if (!m_value.valid()) {
            m_index = Null<size_t>();
        }
        return *this;
    }

    bool operator!=(const SQLResultSetIterator& iter) const {
        return m_index != iter.m_index;
    }

private:
    ResultSet* m_set = nullptr;
    size_t m_index = Null<size_t>();
    TableT m_value;
};

}  // namespace hku