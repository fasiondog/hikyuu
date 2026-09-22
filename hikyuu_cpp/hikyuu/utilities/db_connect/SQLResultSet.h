/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-01-10
 *      Author: fasiondog
 */

#pragma once

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
      m_where(sql),
      m_sql_template("id IN (SELECT id FROM {} WHERE {} {} LIMIT {} OFFSET {}) {}") {
        trim(m_where);
        if (m_where.empty()) {
            m_where = "1=1";
            m_orderby_inner = "ORDER BY id";
            // m_orderby_outer = "";
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
        return m_connect->queryNumber<size_t>(sql, 0);
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
        return n * page_size > total ? n : n + 1;
    }

    /**
     * @brief Get all the data in the given page
     * @param page the given page
     * @return std::vector<TableT> all the valid data sets contained in this page
     */
    std::vector<TableT> getPage(size_t page) {
        std::vector<TableT> result;
        m_connect->batchLoad(
          result, fmt::format(m_sql_template, TableT::getTableName(), m_where, m_orderby_inner,
                              page_size, page * page_size, m_orderby_outer));
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
    TableT get(size_t index) {
        TableT result{Null<TableT>()};
        HKU_IF_RETURN(index == Null<size_t>(), result);

        size_t page = index / page_size;
        if (m_connect && page != m_current_page) {
            m_buffer.clear();
            m_connect->batchLoad(
              m_buffer, fmt::format(fmt::runtime(m_sql_template), TableT::getTableName(), m_where,
                                    m_orderby_inner, page_size, page * page_size, m_orderby_outer));
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
    : m_set(other.m_set), m_index(other.m_index) {}

    SQLResultSetIterator& operator=(const SQLResultSetIterator& other) {
        if (this == &other)
            return *this;
        m_index = other.m_index;
        m_set = other.m_set;
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