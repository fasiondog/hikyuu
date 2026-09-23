/*
 * AsyncDBConnectBase.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-07
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_ASYNCDbCONNECTBASE_H
#define HIKYUU_DB_CONNECT_ASYNCDbCONNECTBASE_H

#include "hikyuu/utilities/Parameter.h"
#include "hikyuu/utilities/Null.h"
#include "hikyuu/utilities/net.h"
#include "DBCondition.h"
#include "AsyncSQLStatementBase.h"
#include "SQLException.h"

namespace hku {

template <class TableT, size_t page_size>
class AsyncSQLResultSet;

/**
 * Base class of the asynchronous database connection
 * @ingroup DBConnect
 *
 * It provides an asynchronous database operation interface based on the boost::asio coroutine.
 * All the I/O intensive operations (such as the query and the execution) return an awaitable, and
 * co_await can be used in a coroutine.
 *
 * @note Not all the interfaces need to be changed into await coroutines, only the I/O intensive
 *       operations need to be made asynchronous
 * @note The synchronous operations such as the transaction control and the configuration access
 * keep the original synchronous interfaces
 */
class HKU_UTILS_API AsyncDBConnectBase : public std::enable_shared_from_this<AsyncDBConnectBase> {
PARAMETER_SUPPORT  // NOSONAR

  public :
  /**
   * Constructor
   * @param param database connection parameters
   */
  explicit AsyncDBConnectBase(const Parameter &param);
    virtual ~AsyncDBConnectBase() = default;

    //-------------------------------------------------------------------------
    // Subclass interface - the asynchronous methods (returning an awaitable)
    //-------------------------------------------------------------------------

    /** The ping operation, used to judge whether it is connected */
    virtual net::awaitable<bool> ping() = 0;

    /** Start a transaction; an exception is thrown on failure */
    virtual net::awaitable<void> transaction() = 0;

    /** Commit the transaction; an exception is thrown on failure */
    virtual net::awaitable<void> commit() = 0;

    /** Roll back the transaction */
    virtual net::awaitable<void> rollback() noexcept = 0;

    /** Execute the SQL without a result */
    virtual net::awaitable<int64_t> exec(const std::string &sql_string) = 0;

    /** Get the AsyncSQLStatement */
    virtual net::awaitable<AsyncSQLStatementPtr> getStatement(const std::string &sql_statement) = 0;

    /** Judge whether the table exists */
    virtual net::awaitable<bool> tableExist(const std::string &tablename) = 0;

    /**
     * Reset the id in the table with an auto-increment id to start from 1
     * @param tablename the table name whose id is to be reset
     * @exception An exception is thrown when the table still contains data
     */
    virtual net::awaitable<void> resetAutoIncrement(const std::string &tablename) = 0;

    //-------------------------------------------------------------------------
    // Template methods - the asynchronous version
    //-------------------------------------------------------------------------

    /**
     * Save or update the table structure bound through TABLE_BIND
     * @param item the record to be saved
     * @param autotrans start a transaction
     */
    template <typename T>
    net::awaitable<void> save(T &item, bool autotrans = true);

    /**
     * Batch saving
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    net::awaitable<void> batchSave(Container &container, bool autotrans = true);

    /**
     * Batch saving; the data in the iterators must be the table model bound through TABLE_BIND
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    net::awaitable<void> batchSave(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * Load the model data into the given model instance
     * @note The query condition should return one record only; if there are multiple query results,
     *       only one is taken
     * @param item the given model instance
     * @param where the query condition, e.g. "id=1"
     */
    template <typename T>
    net::awaitable<void> load(T &item, const std::string &where = "");

    /**
     * Load the model data into the given model instance
     * @note The query condition should return one record only; if there are multiple query results,
     *       only one is taken
     * @param item the given model instance
     * @param cond query condition
     */
    template <typename T>
    net::awaitable<void> load(T &item, const DBCondition &cond);

    /**
     * Load the model data into the given model instance, for the query only
     * @param item the given model instance
     * @param sql the select sql statement of the query condition
     */
    template <typename T>
    net::awaitable<void> loadView(T &item, const std::string &sql);

    /**
     * Batch load the model data into a container
     * @param container the given container
     * @param where query condition
     */
    template <typename Container>
    net::awaitable<void> batchLoad(Container &container, const std::string &where = "");

    /**
     * Batch load the model data into a container
     * @param container the given container
     * @param cond query condition
     */
    template <typename Container>
    net::awaitable<void> batchLoad(Container &container, const DBCondition &cond);

    /**
     * Batch load the model data into a container
     * @param container the given container
     * @param sql the select query statement
     */
    template <typename Container>
    net::awaitable<void> batchLoadView(Container &container, const std::string &sql);

    /**
     * Batch updating
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    net::awaitable<void> batchUpdate(Container &container, bool autotrans = true);

    /**
     * Batch updating
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    net::awaitable<void> batchUpdate(InputIterator first, InputIterator last,
                                     bool autotrans = true);

    /**
     * Batch saving or updating
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    net::awaitable<void> batchSaveOrUpdate(Container &container, bool autotrans = true);

    /**
     * Batch saving or updating
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    net::awaitable<void> batchSaveOrUpdate(InputIterator first, InputIterator last,
                                           bool autotrans = true);

    /**
     * Delete the data satisfying the condition from the given table
     * @param tablename the table name of the data to be deleted
     * @param where the deletion condition
     * @param autotrans start a transaction
     */
    net::awaitable<void> remove(const std::string &tablename, const std::string &where,
                                bool autotrans = true);

    /**
     * Delete the data satisfying the condition from the given table
     * @param tablename the table name of the data to be deleted
     * @param cond the deletion condition
     * @param autotrans start a transaction
     */
    net::awaitable<void> remove(const std::string &tablename, const DBCondition &cond,
                                bool autotrans = true);

    /**
     * Delete
     * @param item the data to be deleted; it is deleted through item.rowid(), and afterwards the
     * rowid is set to invalid
     * @param autotrans start a transaction
     */
    template <typename T>
    net::awaitable<void> remove(T &item, bool autotrans = true);

    /**
     * Batch deleting
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    net::awaitable<void> batchRemove(Container &container, bool autotrans = true);

    /**
     * Batch deleting; the data in the iterators must be the table model bound through TABLE_BIND
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    net::awaitable<void> batchRemove(InputIterator first, InputIterator last,
                                     bool autotrans = true);

    /**
     * Query a single integer, e.g. select count(*) from table
     * @note The sql statement should return a single element only, otherwise an exception is
     * thrown, such as for multiple records or multiple columns
     * @param query query statement
     * @param default_val the default value returned when the query fails. An exception is thrown if
     * it is Null<int>().
     */
    net::awaitable<int> queryInt(const std::string &query, int default_val);

    /**
     * Query the statistical data, e.g. select count(*) from table
     * @note The sql statement should return a single element only, otherwise an exception is
     * thrown, such as for multiple records or multiple columns
     * @param query query statement
     * @param default_val the default value returned when the query fails. An exception is thrown if
     * it is Null<NumberType>().
     */
    template <typename NumberType>
    net::awaitable<NumberType> queryNumber(const std::string &query,
                                           NumberType default_val = Null<NumberType>());

    /**
     * Paged query
     * @tparam TableT the query data structure
     * @tparam page_size the number of the data records per page
     * @return AsyncSQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    AsyncSQLResultSet<TableT, page_size> query();

    /**
     * Paged query
     * @tparam TableT the query data structure
     * @tparam page_size the number of the data records per page
     * @param query query condition
     * @return AsyncSQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    AsyncSQLResultSet<TableT, page_size> query(const std::string &query);

    /**
     * Paged query
     * @tparam TableT the query data structure
     * @tparam page_size the number of the data records per page
     * @param cond query condition
     * @return AsyncSQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    AsyncSQLResultSet<TableT, page_size> query(const DBCondition &cond);

private:
    AsyncDBConnectBase() = delete;
};

/** @ingroup DBConnect */
typedef std::shared_ptr<AsyncDBConnectBase> AsyncDBConnectPtr;

//-------------------------------------------------------------------------
// Implementation of the inline methods
//-------------------------------------------------------------------------

inline AsyncDBConnectBase::AsyncDBConnectBase(const Parameter &param) : m_params(param) {}

inline net::awaitable<int> AsyncDBConnectBase::queryInt(const std::string &query, int default_val) {
    co_return co_await queryNumber<int>(query, default_val);
}

template <typename NumberType>
net::awaitable<NumberType> AsyncDBConnectBase::queryNumber(const std::string &query,
                                                           NumberType default_val) {
    auto st = co_await getStatement(query);
    co_await st->exec();

    if (!(co_await st->moveNext() && st->getNumColumns() == 1)) {
        HKU_CHECK(default_val != Null<NumberType>(), "query doesn't result in exactly 1 element");
        co_return default_val;
    }

    NumberType result = 0;
    st->getColumn(0, result);  // getColumn is a synchronous method

    if (co_await st->moveNext()) {
        HKU_CHECK(default_val != Null<NumberType>(), "query doesn't result in exactly 1 element");
        co_return default_val;
    }

    co_return result;
}

//-------------------------------------------------------------------------
// Implementation of the template methods
//-------------------------------------------------------------------------

template <typename T>
net::awaitable<void> AsyncDBConnectBase::save(T &item, bool autotrans) {
    auto st =
      co_await (item.valid() ? getStatement(T::getUpdateSQL()) : getStatement(T::getInsertSQL()));

    if (autotrans) {
        co_await transaction();
    }

    std::exception_ptr saved_exception;
    try {
        if (item.valid()) {
            item.update(st);
            co_await st->exec();
        } else {
            item.save(st);
            co_await st->exec();
            item.rowid(st->getLastRowid());  // getLastRowid is a synchronous method
        }

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // Handle the rollback outside the try-catch
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // Ignore the rollback exception and keep the original exception
            }
        }
        std::rethrow_exception(saved_exception);
    }
    co_return;
}

template <class Container>
inline net::awaitable<void> AsyncDBConnectBase::batchSave(Container &container, bool autotrans) {
    co_await batchSave(container.begin(), container.end(), autotrans);
}

template <class InputIterator>
net::awaitable<void> AsyncDBConnectBase::batchSave(InputIterator first, InputIterator last,
                                                   bool autotrans) {
    size_t count = std::distance(first, last);
    if (count == 0) {
        co_return;
    }

    auto st = co_await getStatement(InputIterator::value_type::getInsertSQL());

    if (autotrans) {
        co_await transaction();
    }

    std::exception_ptr saved_exception;
    try {
        for (InputIterator iter = first; iter != last; ++iter) {
            iter->save(st);
            co_await st->exec();
            iter->rowid(st->getLastRowid());  // getLastRowid is a synchronous method
        }

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // Handle the rollback outside the try-catch
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // Ignore the rollback exception and keep the original exception
            }
        }
        std::rethrow_exception(saved_exception);
    }
    co_return;
}

template <typename T>
net::awaitable<void> AsyncDBConnectBase::load(T &item, const std::string &where) {
    std::ostringstream sql;
    if (where != "") {
        sql << T::getSelectSQL() << " where " << where << " limit 1";
    } else {
        sql << T::getSelectSQL() << " limit 1";
    }

    auto st = co_await getStatement(sql.str());
    co_await st->exec();

    if (co_await st->moveNext()) {
        item.load(st);
    }
    co_return;
}

template <typename T>
net::awaitable<void> AsyncDBConnectBase::load(T &item, const DBCondition &cond) {
    co_await load(item, cond.str());
    co_return;
}

template <typename Container>
net::awaitable<void> AsyncDBConnectBase::batchLoad(Container &container, const std::string &where) {
    std::ostringstream sql;
    if (where != "") {
        sql << Container::value_type::getSelectSQL() << " where " << where;
    } else {
        sql << Container::value_type::getSelectSQL();
    }

    auto st = co_await getStatement(sql.str());
    co_await st->exec();

    while (co_await st->moveNext()) {
        typename Container::value_type tmp;
        tmp.load(st);
        container.push_back(tmp);
    }
}

template <typename Container>
net::awaitable<void> AsyncDBConnectBase::batchLoad(Container &container, const DBCondition &cond) {
    co_await batchLoad(container, cond.str());
}

template <typename T>
net::awaitable<void> AsyncDBConnectBase::loadView(T &item, const std::string &sql) {
    auto st = co_await getStatement(sql);
    co_await st->exec();

    if (co_await st->moveNext()) {
        item.load(st);
    }
    co_return;
}

template <typename Container>
net::awaitable<void> AsyncDBConnectBase::batchLoadView(Container &container,
                                                       const std::string &sql) {
    auto st = co_await getStatement(sql);
    co_await st->exec();

    while (co_await st->moveNext()) {
        typename Container::value_type tmp;
        tmp.load(st);
        container.push_back(tmp);
    }
}

template <class Container>
inline net::awaitable<void> AsyncDBConnectBase::batchUpdate(Container &container, bool autotrans) {
    co_await batchUpdate(container.begin(), container.end(), autotrans);
}

template <class InputIterator>
net::awaitable<void> AsyncDBConnectBase::batchUpdate(InputIterator first, InputIterator last,
                                                     bool autotrans) {
    size_t count = std::distance(first, last);
    if (count == 0) {
        co_return;
    }

    auto st = co_await getStatement(InputIterator::value_type::getUpdateSQL());

    if (autotrans) {
        co_await transaction();
    }

    std::exception_ptr saved_exception;
    try {
        for (InputIterator iter = first; iter != last; ++iter) {
            iter->update(st);
            co_await st->exec();
        }

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // Handle the rollback outside the try-catch
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // Ignore the rollback exception and keep the original exception
            }
        }
        std::rethrow_exception(saved_exception);
    }
    co_return;
}

template <class InputIterator>
net::awaitable<void> AsyncDBConnectBase::batchSaveOrUpdate(InputIterator first, InputIterator last,
                                                           bool autotrans) {
    std::vector<typename InputIterator::value_type> save_list;
    std::vector<typename InputIterator::value_type> update_list;

    for (auto iter = first; iter != last; ++iter) {
        if (iter->valid()) {
            update_list.push_back(*iter);
        } else {
            save_list.push_back(*iter);
        }
    }

    co_await batchSave(save_list.begin(), save_list.end(), autotrans);
    co_await batchUpdate(update_list.begin(), update_list.end(), autotrans);
}

template <class Container>
inline net::awaitable<void> AsyncDBConnectBase::batchSaveOrUpdate(Container &container,
                                                                  bool autotrans) {
    co_await batchSaveOrUpdate(container.begin(), container.end(), autotrans);
}

template <typename T>
net::awaitable<void> AsyncDBConnectBase::remove(T &item, bool autotrans) {
    HKU_CHECK(item.valid(), "Invalid item, id is 0!");

    auto st = co_await getStatement(
      fmt::format("delete from {} where id={}", T::getTableName(), item.rowid()));

    if (autotrans) {
        co_await transaction();
    }

    std::exception_ptr saved_exception;
    try {
        co_await st->exec();

        if (autotrans) {
            co_await commit();
        }
        item.rowid(0);
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // Handle the rollback outside the try-catch
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // Ignore the rollback exception and keep the original exception
            }
        }
        std::rethrow_exception(saved_exception);
    }
    co_return;
}

template <class Container>
inline net::awaitable<void> AsyncDBConnectBase::batchRemove(Container &container, bool autotrans) {
    co_await batchRemove(container.begin(), container.end(), autotrans);
}

template <class InputIterator>
net::awaitable<void> AsyncDBConnectBase::batchRemove(InputIterator first, InputIterator last,
                                                     bool autotrans) {
    size_t count = std::distance(first, last);
    if (count == 0) {
        co_return;
    }

    if (autotrans) {
        co_await transaction();
    }

    std::exception_ptr saved_exception;
    try {
        for (InputIterator iter = first; iter != last; ++iter) {
            co_await remove(*iter, false);  // The outer layer has handled the transaction
        }

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // Handle the rollback outside the try-catch
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // Ignore the rollback exception and keep the original exception
            }
        }
        std::rethrow_exception(saved_exception);
    }
    co_return;
}

inline net::awaitable<void> AsyncDBConnectBase::remove(const std::string &tablename,
                                                       const std::string &where, bool autotrans) {
    if (autotrans) {
        co_await transaction();
    }

    std::string sql = (where == "" || where == "1=1")
                        ? fmt::format("delete from {}", tablename, where)
                        : (fmt::format("delete from {} where {}", tablename, where));

    std::exception_ptr saved_exception;
    try {
        co_await exec(sql);

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // Handle the rollback outside the try-catch
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // Ignore the rollback exception and keep the original exception
            }
        }
        std::rethrow_exception(saved_exception);
    }
    co_return;
}

inline net::awaitable<void> AsyncDBConnectBase::remove(const std::string &tablename,
                                                       const DBCondition &cond, bool autotrans) {
    co_await remove(tablename, cond.str(), autotrans);
}

template <typename TableT, size_t page_size>
AsyncSQLResultSet<TableT, page_size> AsyncDBConnectBase::query() {
    return AsyncSQLResultSet<TableT, page_size>(shared_from_this(), "");
}

template <typename TableT, size_t page_size>
AsyncSQLResultSet<TableT, page_size> AsyncDBConnectBase::query(const std::string &query) {
    return AsyncSQLResultSet<TableT, page_size>(shared_from_this(), query);
}

template <typename TableT, size_t page_size>
AsyncSQLResultSet<TableT, page_size> AsyncDBConnectBase::query(const DBCondition &cond) {
    return AsyncSQLResultSet<TableT, page_size>(shared_from_this(), cond.str());
}

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_ASYNCDbCONNECTBASE_H */
