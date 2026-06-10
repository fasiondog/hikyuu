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
 * 异步 SQL 查询结果集
 * @tparam TableT 数据结构
 * @tparam page_size 每页包含的数据数量
 * @ingroup DBConnect
 *
 * 提供基于 boost::asio 协程的异步分页查询接口。
 * 所有 I/O 操作（如 size、getPage、迭代器访问）都返回 awaitable。
 *
 * @note 保持与 SQLResultSet 相同的 API 设计，仅将 I/O 操作改为异步
 * @note 适用于高并发场景下的分页查询
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
     * 构建新的异步分页查询结果实例
     * @param connect 异步数据库连接
     * @param sql 查询条件
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

    /** 获取其数据库连接 */
    const AsyncDBConnectPtr& getConnect() const {
        return m_connect;
    }

    using const_iterator = AsyncSQLResultSetIterator<TableT, page_size>;
    using iterator = AsyncSQLResultSetIterator<TableT, page_size>;

    /**
     * @brief 获取起始迭代器
     * @return 异步迭代器
     */
    net::awaitable<const_iterator> cbegin() {
        co_return const_iterator(this, 0);
    }

    /**
     * @brief 获取结束迭代器
     * @return 异步迭代器
     */
    const_iterator cend() {
        return const_iterator(this, Null<size_t>());
    }

    /**
     * @brief 获取起始迭代器
     * @return 异步迭代器
     */
    net::awaitable<iterator> begin() {
        co_return iterator(this, 0);
    }

    /**
     * @brief 获取结束迭代器
     * @return 异步迭代器
     */
    iterator end() {
        return iterator(this, Null<size_t>());
    }

    /**
     * @brief 获取当前时刻数据集大小
     * @note 数据集大小会根据当前的数据库内容发生变化，并非一直不变
     * @return size_t 数据集大小
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
     * @brief 当前数据集是否为空
     * @return true 空
     * @return false 非空
     */
    net::awaitable<bool> empty() const {
        size_t sz = co_await size();
        co_return sz == 0;
    }

    /**
     * @brief 获取当前数据集分页数量
     * @note 仅为调用时刻获取的相应数据集分页数量
     * @return size_t 分页数量
     */
    net::awaitable<size_t> getPageCount() {
        size_t total = co_await size();
        size_t n = total / page_size;
        co_return n* page_size >= total ? n : n + 1;
    }

    /**
     * @brief 获取指定页中的全部数据
     * @param page 指定页
     * @return std::vector<TableT> 该页包含的所有有效数据集
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
     * @brief 获取指定索引的数据
     * @param index 索引位置
     * @return TableT 数据对象
     */
    net::awaitable<TableT> at(size_t index) {
        TableT result = co_await get(index);
        HKU_CHECK_THROW(result.valid(), std::out_of_range, "Index is over");
        co_return result;
    }

    /**
     * @brief 获取指定索引的数据（下标操作符）
     * @param index 索引位置
     * @return TableT 数据对象
     */
    net::awaitable<TableT> operator_bracket(size_t index) {
        co_return co_await get(index);
    }

private:
    /**
     * @brief 内部获取方法
     * @param index 索引位置
     * @return TableT 数据对象
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
        // 注意：构造函数中不能直接使用 co_await，需要在外部初始化
    }

    /**
     * @brief 初始化迭代器值
     * @note 必须在协程中调用此方法完成初始化
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
     * @brief 前置递增运算符
     * @return 新的迭代器
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
