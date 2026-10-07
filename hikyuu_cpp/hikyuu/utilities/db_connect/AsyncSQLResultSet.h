/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2026-05-07
 *      Author: fasiondog
 */

#pragma once

#include <algorithm>
#include <iterator>
#include "hikyuu/utilities/arithmetic.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/osdef.h"
#include "../net.h"
#include "AsyncDBConnectBase.h"

namespace hku {

template <class TableT, size_t page_size>
class AsyncSQLResultSetIterator;

/**
 * Asynchronous SQL query result set
 * @tparam TableT data structure
 * @tparam page_size the number of the data contained in every page
 * @ingroup DBConnect
 *
 * It provides an asynchronous paged query interface based on the boost::asio coroutine.
 * All the I/O operations (such as size, getPage and the iterator access) return an awaitable.
 *
 * @note It keeps the same API design as SQLResultSet, only the I/O operations are made asynchronous
 * @note It is suitable for the paged query under the high concurrency scenarios
 *
 * @note Precondition: same as SQLResultSet — TableT must be bound through the TABLE_BIND macros,
 *       which make the ORM layer rely on an integer primary key column named "id"; the flat paged
 *       query orders by it (`... ORDER BY id LIMIT/OFFSET`), so a table without an "id" column is
 *       not supported by any query path (paged or not)
 */
template <class TableT, size_t page_size = 100>
class AsyncSQLResultSet {
    friend class AsyncSQLResultSetIterator<TableT, page_size>;

public:
#if CPP_STANDARD >= CPP_STANDARD_17
    static constexpr int PSIZE = page_size;
#else
    static const int PSIZE = page_size;
#endif

    AsyncSQLResultSet() = default;

    /**
     * Build a new asynchronous paged query result instance
     * @param connect asynchronous database connection
     * @param sql query condition
     */
    AsyncSQLResultSet(const AsyncDBConnectPtr& connect, const std::string& sql)
    : m_connect(connect), m_sql_template("SELECT * FROM {} WHERE {} {} LIMIT {} OFFSET {}") {
        // the plain path: the clauses can only be located in the text the caller wrote
        WhereParts parts = splitWhereParts(sql);
        _setup(parts.where, parts.orderBy, parts.limit, BoundValues{});
    }

    /**
     * Build a new asynchronous paged query result instance from a condition
     * @param connect asynchronous database connection
     * @param cond the query condition, whose values stay bound to placeholders
     */
    AsyncSQLResultSet(const AsyncDBConnectPtr& connect, const DBCondition& cond)
    : m_connect(connect), m_sql_template("SELECT * FROM {} WHERE {} {} LIMIT {} OFFSET {}") {
        // a condition already carries its parts, nothing has to be parsed out of the text
        _setup(cond.sql(), cond.getOrderBy(), cond.getLimit(), cond.params());
    }

    /** Get its database connection */
    const AsyncDBConnectPtr& getConnect() const {
        return m_connect;
    }

    using const_iterator = AsyncSQLResultSetIterator<TableT, page_size>;
    using iterator = AsyncSQLResultSetIterator<TableT, page_size>;

    /**
     * @brief Get the begin iterator
     * @return asynchronous iterator
     */
    net::awaitable<const_iterator> cbegin() {
        co_return const_iterator(this, 0);
    }

    /**
     * @brief Get the end iterator
     * @return asynchronous iterator
     */
    const_iterator cend() {
        return const_iterator(this, Null<size_t>());
    }

    /**
     * @brief Get the begin iterator
     * @return asynchronous iterator
     */
    net::awaitable<iterator> begin() {
        co_return iterator(this, 0);
    }

    /**
     * @brief Get the end iterator
     * @return asynchronous iterator
     */
    iterator end() {
        return iterator(this, Null<size_t>());
    }

    /**
     * @brief Get the data set size at the current moment
     * @note The data set size changes with the current database content, it is not always constant
     * @return size_t the data set size
     */
    net::awaitable<size_t> size() const {
        if (!m_connect) {
            co_return 0;
        }
        std::string sql =
          fmt::format("select count(1) from {} where {}", TableT::getTableName(), m_where);
        size_t total = co_await m_connect->queryNumber<size_t>(sql, 0, m_params);
        // the row limit of the condition caps the reported size, the pages are cut the same way
        co_return m_limit >= 0 ? std::min(total, static_cast<size_t>(m_limit)) : total;
    }

    /**
     * @brief Whether the current data set is empty
     * @return true empty
     * @return false not empty
     */
    net::awaitable<bool> empty() const {
        size_t sz = co_await size();
        co_return sz == 0;
    }

    /**
     * @brief Get the number of the pages of the current data set
     * @note It is the number of the pages of the corresponding data set obtained at the calling
     * moment only
     * @return size_t the number of the pages
     */
    net::awaitable<size_t> getPageCount() {
        size_t total = co_await size();
        size_t n = total / page_size;
        co_return n* page_size >= total ? n : n + 1;
    }

    /**
     * @brief Get all the data in the given page
     * @param page the given page
     * @return std::vector<TableT> all the valid data sets contained in this page
     */
    net::awaitable<std::vector<TableT>> getPage(size_t page) {
        if (!_connectOrPage(page)) {
            co_return std::vector<TableT>{};
        }
        co_return co_await _loadPage(page);
    }

    /**
     * @brief Get the data of the given index
     * @param index the index position
     * @return TableT the data object
     */
    net::awaitable<TableT> at(size_t index) {
        TableT result = co_await get(index);
        HKU_CHECK_THROW(result.valid(), std::out_of_range, "Index is over");
        co_return result;
    }

    /**
     * @brief Get the data of the given index (the subscript operator)
     * @param index the index position
     * @return TableT the data object
     */
    net::awaitable<TableT> operator_bracket(size_t index) {
        co_return co_await get(index);
    }

private:
    /** Prepare the parts of the query: the filter, the order-by of the select */
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
            m_orderby_outer = m_orderby_inner;
        }
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

    /** Whether the given page can be fetched at all */
    bool _connectOrPage(size_t page) const {
        return m_connect && _pageLimit(page) > 0;
    }

    /** The full select statement of the given page, the values still bound to placeholders */
    std::string _selectSQL(size_t page) const {
        return fmt::format(fmt::runtime(m_sql_template), TableT::getTableName(), m_where,
                           m_orderby_inner, _pageLimit(page), page * page_size);
    }

    /** Run the select of the given page and collect its rows */
    net::awaitable<std::vector<TableT>> _loadPage(size_t page) {
        std::vector<TableT> result;
        auto st = co_await m_connect->getStatementWithParams(_selectSQL(page), m_params);
        co_await st->exec();

        while (co_await st->moveNext()) {
            TableT tmp;
            tmp.load(st);
            result.push_back(tmp);
        }
        co_return result;
    }

    /**
     * @brief The internal get method
     * @param index the index position
     * @return TableT the data object
     */
    net::awaitable<TableT> get(size_t index) {
        TableT result{Null<TableT>()};
        if (index == Null<size_t>()) {
            co_return result;
        }

        size_t page = index / page_size;
        if (m_connect && page != m_current_page) {
            m_buffer.clear();
            if (_connectOrPage(page)) {
                m_buffer = co_await _loadPage(page);
            }
            m_current_page = page;
        }

        if (m_buffer.empty()) {
            co_return result;
        }

        size_t pos = index - page * page_size;
        if (pos >= m_buffer.size()) {
            co_return result;
        }

        result = m_buffer[index - page * page_size];
        co_return result;
    }

private:
    AsyncDBConnectPtr m_connect;
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
class AsyncSQLResultSetIterator {
public:
    using ResultSet = AsyncSQLResultSet<TableT, page_size>;

    AsyncSQLResultSetIterator() = default;
    ~AsyncSQLResultSetIterator() = default;

    explicit AsyncSQLResultSetIterator(ResultSet* result_set, size_t index)
    : m_set(result_set), m_index(index) {
        // Note: co_await cannot be used directly in the constructor, it needs to be initialized
        // outside
    }

    /**
     * @brief Initialize the iterator values
     * @note This method must be called in a coroutine to complete the initialization
     */
    net::awaitable<void> init() {
        if (m_index != Null<size_t>()) {
            m_value = co_await m_set->get(m_index);
            if (!m_value.valid()) {
                m_index = Null<size_t>();
            }
        }
        co_return;
    }

    AsyncSQLResultSetIterator(const AsyncSQLResultSetIterator& other)
    : m_set(other.m_set), m_index(other.m_index), m_value(other.m_value) {}

    AsyncSQLResultSetIterator& operator=(const AsyncSQLResultSetIterator& other) {
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

    /**
     * @brief Prefix increment operator
     * @return the new iterator
     */
    net::awaitable<AsyncSQLResultSetIterator> operator_pre_increment() {
        HKU_CHECK_THROW(m_index != Null<size_t>(), std::logic_error,
                        "Cannot increment an end iterator.");
        m_index++;
        m_value = co_await m_set->get(m_index);
        if (!m_value.valid()) {
            m_index = Null<size_t>();
        }
        co_return *this;
    }

    bool operator!=(const AsyncSQLResultSetIterator& iter) const {
        return m_index != iter.m_index;
    }

private:
    ResultSet* m_set = nullptr;
    size_t m_index = Null<size_t>();
    TableT m_value;
};

}  // namespace hku
