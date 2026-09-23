/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2026-05-07
 *      Author: fasiondog
 */

#pragma once

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
    : m_connect(connect),
      m_where(sql),
      m_sql_template("SELECT * FROM {} WHERE {} {} LIMIT {} OFFSET {}") {
        trim(m_where);
        if (m_where.empty()) {
            m_where = "1=1";
            m_orderby_inner = "ORDER BY id";
            return;
        }

        std::string tmp = utf8_to_upper(m_where);
        size_t pos = tmp.rfind("ORDER");
        if (pos != std::string::npos) {
            m_orderby_inner = fmt::format("{}, id ASC", m_where.substr(pos));
            m_orderby_outer = m_orderby_inner;
            m_where = m_where.erase(pos, std::string::npos);
        } else {
            m_orderby_inner = "ORDER BY id";
        }
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
        co_return co_await m_connect->queryNumber<size_t>(sql, 0);
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
        std::vector<TableT> result;
        std::string sql = fmt::format(fmt::runtime(m_sql_template), TableT::getTableName(), m_where,
                                      m_orderby_inner, page_size, page * page_size);

        auto st = co_await m_connect->getStatement(sql);
        co_await st->exec();

        while (co_await st->moveNext()) {
            TableT tmp;
            tmp.load(st);
            result.push_back(tmp);
        }

        co_return result;
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
            std::string sql = fmt::format(fmt::runtime(m_sql_template), TableT::getTableName(),
                                          m_where, m_orderby_inner, page_size, page * page_size);

            auto st = co_await m_connect->getStatement(sql);
            co_await st->exec();

            while (co_await st->moveNext()) {
                TableT tmp;
                tmp.load(st);
                m_buffer.push_back(tmp);
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
