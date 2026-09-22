/*
 * DBConnectBase.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-1
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_DBCONNECTBASE_H
#define HIKYUU_DB_CONNECT_DBCONNECTBASE_H

#include "../../utilities/Parameter.h"
#include "../Null.h"
#include "DBCondition.h"
#include "SQLStatementBase.h"
#include "SQLException.h"

namespace hku {

template <class TableT, size_t page_size>
class SQLResultSet;

/**
 * Base class of the database connection
 * @ingroup DBConnect
 */
class HKU_UTILS_API DBConnectBase : public std::enable_shared_from_this<DBConnectBase> {
PARAMETER_SUPPORT  // NOSONAR

  public :
  /**
   * Constructor
   * @param param database connection parameters
   */
  explicit DBConnectBase(const Parameter &param);
    virtual ~DBConnectBase() = default;

    //-------------------------------------------------------------------------
    // Subclass interface
    //-------------------------------------------------------------------------

    /** The ping operation, used to judge whether it is connected */
    virtual bool ping() = 0;

    /** Start a transaction; an exception is thrown on failure */
    virtual void transaction() = 0;

    /** Commit the transaction; an exception is thrown on failure */
    virtual void commit() = 0;

    /** Roll back the transaction */
    virtual void rollback() noexcept = 0;

    /** Execute the SQL without a result */
    virtual int64_t exec(const std::string &sql_string) = 0;

    /** Get the SQLStatement */
    virtual SQLStatementPtr getStatement(const std::string &sql_statement) = 0;

    /** Judge whether the table exists */
    virtual bool tableExist(const std::string &tablename) = 0;

    /**
     * Reset the id in the table with an auto-increment id to start from 1
     * @param tablename the table name whose id is to be reset
     * @exception An exception is thrown when the table still contains data
     */
    virtual void resetAutoIncrement(const std::string &tablename) = 0;

    //-------------------------------------------------------------------------
    // Template methods
    //-------------------------------------------------------------------------
    /**
     * Save or update the table structure bound through TABLE_BIND
     * It can be saved directly by the driver, the example is as follows:
     * @code
     * struct TTT {
     *   TABLE_BIND(TTT, ttt_table, age, name)
     *
     *       int age;
     *       string name;
     *   public:
     *       void save(const DBConnectPtr& driver) const {
     *           SQLStatementPtr st = driver->getStatement("insert into ttt (name, age) values
     * (?,?)"); st->bind(0, name, age); st->exec();
     *       }
     *   };
     *
     *   TEST_CASE("test_temp", "temp") {
     *       Parameter param;
     *       param.set<string>("db", TST_DATA("test.db"));
     *       DBConnectPtr driver = make_shared<SQLiteConnect>(param);
     *       driver->exec("create table ttt (name, age)");
     *       TTT a;
     *       a.name = "TTT";
     *       a.age = 11;
     *       driver->save(a);
     *   }
     * @endcode
     * @param item the record to be saved
     * @param autotrans start a transaction
     */
    template <typename T>
    void save(T &item, bool autotrans = true);

    /**
     * Batch saving
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    void batchSave(Container &container, bool autotrans = true);

    /**
     * Batch saving; the data in the iterators must be the table model bound through TABLE_BIND
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    void batchSave(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * Load the model data into the given model instance
     * @note The query condition should return one record only; if there are multiple query results,
     *       only one is taken
     * @param item the given model instance
     * @param where the query condition, e.g. "id=1"
     */
    template <typename T>
    void load(T &item, const std::string &where = "");

    /**
     * Load the model data into the given model instance
     * @note The query condition should return one record only; if there are multiple query results,
     *       only one is taken
     * @param item the given model instance
     * @param cond the query condition, e.g. "id=1"
     */
    template <typename T>
    void load(T &item, const DBCondition &cond);

    /**
     * Load the model data into the given model instance, for the query only
     * @param item the given model instance
     * @param sql the select sql statement of the query condition
     */
    template <typename T>
    void loadView(T &item, const std::string &sql);

    /**
     * Batch load the model data into a container (a container supporting push_back, such as vector
     * and list)
     * @param container the given container
     * @param where query condition
     */
    template <typename Container>
    void batchLoad(Container &container, const std::string &where = "");

    /**
     * Batch load the model data into a container (a container supporting push_back, such as vector
     * and list)
     * @param container the given container
     * @param cond query condition
     */
    template <typename Container>
    void batchLoad(Container &container, const DBCondition &cond);

    /**
     * Batch load the model data into a container (a container supporting push_back, such as vector
     * and list)
     * @param container the given container
     * @param sql the select query statement
     */
    template <typename Container>
    void batchLoadView(Container &container, const std::string &sql);

    /**
     * Batch updating
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    void batchUpdate(Container &container, bool autotrans = true);

    /**
     * Batch updating
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    void batchUpdate(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * Batch saving or updating
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    void batchSaveOrUpdate(Container &container, bool autotrans = true);

    /**
     * Batch saving or updating
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    void batchSaveOrUpdate(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * Delete the data satisfying the condition from the given table, i.e. delete by the given
     * condition
     * @param tablename the table name of the data to be deleted
     * @param where the deletion condition
     * @param autotrans start a transaction
     */
    void remove(const std::string &tablename, const std::string &where, bool autotrans = true);

    /**
     * Delete the data satisfying the condition from the given table, i.e. delete by the given
     * condition
     * @param tablename the table name of the data to be deleted
     * @param cond the deletion condition
     * @param autotrans start a transaction
     */
    void remove(const std::string &tablename, const DBCondition &cond, bool autotrans = true);

    /**
     * Delete
     * @param item the data to be deleted; it is deleted through item.rowid(), and afterwards the
     * rowid is set to invalid
     * @param autotrans start a transaction
     */
    template <typename T>
    void remove(T &item, bool autotrans = true);

    /**
     * Batch deleting
     * @param container a container with an iterator
     * @param autotrans start a transaction
     */
    template <class Container>
    void batchRemove(Container &container, bool autotrans = true);

    /**
     * Batch deleting; the data in the iterators must be the table model bound through TABLE_BIND
     * @param first the iterator start point
     * @param last the iterator end point
     * @param autotrans start a transaction
     */
    template <class InputIterator>
    void batchRemove(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * Query a single integer, e.g. select count(*) from table
     * @note The sql statement should return a single element only, otherwise an exception is
     * thrown, such as for multiple records or multiple columns
     * @param query query statement
     * @param default_val the default value returned when the query fails. An exception is thrown if
     * it is Null<int>().
     */
    int queryInt(const std::string &query, int default_val);

    /**
     * Query the statistical data, e.g. select count(*) from table
     * @note The sql statement should return a single element only, otherwise an exception is
     * thrown, such as for multiple records or multiple columns
     * @param query query statement
     * @param default_val the default value returned when the query fails. An exception is thrown if
     * it is Null<NumberType>().
     */
    template <typename NumberType>
    NumberType queryNumber(const std::string &query, NumberType default_val = Null<NumberType>());

    /**
     * Paged query
     * @tparam TableT the query data structure
     * @tparam page_size the number of the data records per page
     * @return SQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    SQLResultSet<TableT, page_size> query();

    /**
     * Paged query
     * @tparam TableT the query data structure
     * @tparam page_size the number of the data records per page
     * @param query query condition
     * @return SQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    SQLResultSet<TableT, page_size> query(const std::string &query);

    /**
     * Paged query
     * @tparam TableT the query data structure
     * @tparam page_size the number of the data records per page
     * @param cond query condition
     * @return SQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    SQLResultSet<TableT, page_size> query(const DBCondition &cond);

private:
    DBConnectBase() = delete;
};

/** @ingroup DBConnect */
typedef std::shared_ptr<DBConnectBase> DBConnectPtr;

//-------------------------------------------------------------------------
// Implementation of the inline methods
//-------------------------------------------------------------------------

inline DBConnectBase::DBConnectBase(const Parameter &param) : m_params(param) {}

inline int DBConnectBase::queryInt(const std::string &query, int default_val) {
    return queryNumber<int>(query, default_val);
}

template <typename NumberType>
NumberType DBConnectBase::queryNumber(const std::string &query, NumberType default_val) {
    SQLStatementPtr st = getStatement(query);
    st->exec();
    if (!(st->moveNext() && st->getNumColumns() == 1)) {
        HKU_CHECK(default_val != Null<NumberType>(), "query doesn't result in exactly 1 element");
        return default_val;
    }
    NumberType result = 0;
    st->getColumn(0, result);
    if (st->moveNext()) {
        HKU_CHECK(default_val != Null<NumberType>(), "query doesn't result in exactly 1 element");
        return default_val;
    }
    return result;
}

//-------------------------------------------------------------------------
// Implementation of the template methods
//-------------------------------------------------------------------------

template <typename TableT, size_t page_size>
SQLResultSet<TableT, page_size> DBConnectBase::query() {
    return SQLResultSet<TableT, page_size>(shared_from_this(), "");
}

template <typename TableT, size_t page_size>
SQLResultSet<TableT, page_size> DBConnectBase::query(const std::string &query) {
    return SQLResultSet<TableT, page_size>(shared_from_this(), query);
}

template <typename TableT, size_t page_size>
SQLResultSet<TableT, page_size> DBConnectBase::query(const DBCondition &cond) {
    return SQLResultSet<TableT, page_size>(shared_from_this(), cond.str());
}

template <typename T>
void DBConnectBase::save(T &item, bool autotrans) {
    SQLStatementPtr st =
      item.valid() ? getStatement(T::getUpdateSQL()) : getStatement(T::getInsertSQL());
    if (autotrans) {
        transaction();
    }

    try {
        if (item.valid()) {
            item.update(st);
            st->exec();
        } else {
            item.save(st);
            st->exec();
            item.rowid(st->getLastRowid());
        }

        if (autotrans) {
            commit();
        }
    } catch (::hku::SQLException &e) {
        if (autotrans) {
            rollback();
        }
        SQL_THROW(e.errcode(), "failed save! sql: {}! {}", st->getSqlString(), e.what());
    } catch (std::exception &e) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed save! sql: {}! {}", st->getSqlString(), e.what());
    } catch (...) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed save! sql: {}! Unknown error!", st->getSqlString());
    }
}

template <class Container>
inline void DBConnectBase::batchSave(Container &container, bool autotrans) {
    batchSave(container.begin(), container.end(), autotrans);
}

template <class InputIterator>
void DBConnectBase::batchSave(InputIterator first, InputIterator last, bool autotrans) {
    size_t count = std::distance(first, last);
    HKU_IF_RETURN(count == 0, void());

    SQLStatementPtr st = getStatement(InputIterator::value_type::getInsertSQL());
    if (autotrans) {
        transaction();
    }

    try {
        for (InputIterator iter = first; iter != last; ++iter) {
            iter->save(st);
            st->exec();
            iter->rowid(st->getLastRowid());
        }

        if (autotrans) {
            commit();
        }
    } catch (::hku::SQLException &e) {
        if (autotrans) {
            rollback();
        }
        SQL_THROW(e.errcode(), "failed batch save! sql: {}! {}", st->getSqlString(), e.what());
    } catch (std::exception &e) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed batch save! sql: {}! {}", st->getSqlString(), e.what());
    } catch (...) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed batch save! sql: {}! Unknown error!", st->getSqlString());
    }
}

template <typename T>
void DBConnectBase::load(T &item, const std::string &where) {
    std::ostringstream sql;
    if (where != "") {
        sql << T::getSelectSQL() << " where " << where << " limit 1";
    } else {
        sql << T::getSelectSQL() << " limit 1";
    }
    SQLStatementPtr st = getStatement(sql.str());
    st->exec();
    if (st->moveNext()) {
        item.load(st);
    }
}

template <typename T>
void DBConnectBase::load(T &item, const DBCondition &cond) {
    load(item, cond.str());
}

template <typename Container>
void DBConnectBase::batchLoad(Container &container, const std::string &where) {
    std::ostringstream sql;
    if (where != "") {
        sql << Container::value_type::getSelectSQL() << " where " << where;
    } else {
        sql << Container::value_type::getSelectSQL();
    }
    SQLStatementPtr st = getStatement(sql.str());
    st->exec();
    while (st->moveNext()) {
        typename Container::value_type tmp;
        tmp.load(st);
        container.push_back(tmp);
    }
}

template <typename Container>
void DBConnectBase::batchLoad(Container &container, const DBCondition &cond) {
    batchLoad(container, cond.str());
}

template <typename T>
void DBConnectBase::loadView(T &item, const std::string &sql) {
    SQLStatementPtr st = getStatement(sql);
    st->exec();
    if (st->moveNext()) {
        item.load(st);
    }
}

template <typename Container>
void DBConnectBase::batchLoadView(Container &container, const std::string &sql) {
    SQLStatementPtr st = getStatement(sql);
    st->exec();
    while (st->moveNext()) {
        typename Container::value_type tmp;
        tmp.load(st);
        container.push_back(tmp);
    }
}

template <class Container>
inline void DBConnectBase::batchUpdate(Container &container, bool autotrans) {
    batchUpdate(container.begin(), container.end(), autotrans);
}

template <class InputIterator>
void DBConnectBase::batchUpdate(InputIterator first, InputIterator last, bool autotrans) {
    size_t count = std::distance(first, last);
    HKU_IF_RETURN(count == 0, void());

    SQLStatementPtr st = getStatement(InputIterator::value_type::getUpdateSQL());
    if (autotrans) {
        transaction();
    }

    try {
        for (InputIterator iter = first; iter != last; ++iter) {
            iter->update(st);
            st->exec();
        }

        if (autotrans) {
            commit();
        }
    } catch (::hku::SQLException &e) {
        if (autotrans) {
            rollback();
        }
        SQL_THROW(e.errcode(), "failed batch save! sql: {}! {}", st->getSqlString(), e.what());
    } catch (std::exception &e) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed batch update! sql: {}! {}", st->getSqlString(), e.what());
    } catch (...) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed batch save! sql: {}! Unknown error!", st->getSqlString());
    }
}

template <class InputIterator>
void DBConnectBase::batchSaveOrUpdate(InputIterator first, InputIterator last, bool autotrans) {
    std::vector<typename InputIterator::value_type> save_list;
    std::vector<typename InputIterator::value_type> update_list;
    for (auto iter = first; iter != last; ++iter) {
        if (iter->valid()) {
            update_list.push_back(*iter);
        } else {
            save_list.push_back(*iter);
        }
    }

    batchSave(save_list.begin(), save_list.end(), autotrans);
    batchUpdate(update_list.begin(), update_list.end(), autotrans);
}

template <class Container>
inline void DBConnectBase::batchSaveOrUpdate(Container &container, bool autotrans) {
    batchSaveOrUpdate(container.begin(), container.end(), autotrans);
}

template <typename T>
void DBConnectBase::remove(T &item, bool autotrans) {
    HKU_CHECK(item.valid(), "Invalid item, id is 0!");
    SQLStatementPtr st =
      getStatement(fmt::format("delete from {} where id={}", T::getTableName(), item.rowid()));
    if (autotrans) {
        transaction();
    }

    try {
        st->exec();
        if (autotrans) {
            commit();
        }
        item.rowid(0);
    } catch (::hku::SQLException &e) {
        if (autotrans) {
            rollback();
        }
        SQL_THROW(e.errcode(), "failed delete! sql: {}! {}", st->getSqlString(), e.what());
    } catch (std::exception &e) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed delete! sql: {}! {}", st->getSqlString(), e.what());
    } catch (...) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed delete! sql: {}! Unknown error!", st->getSqlString());
    }
}

template <class Container>
inline void DBConnectBase::batchRemove(Container &container, bool autotrans) {
    batchRemove(container.begin(), container.end(), autotrans);
}

template <class InputIterator>
void DBConnectBase::batchRemove(InputIterator first, InputIterator last, bool autotrans) {
    size_t count = std::distance(first, last);
    HKU_IF_RETURN(count == 0, void());

    if (autotrans) {
        transaction();
    }

    try {
        for (InputIterator iter = first; iter != last; ++iter) {
            remove(*iter);
        }

        if (autotrans) {
            commit();
        }
    } catch (::hku::SQLException &e) {
        if (autotrans) {
            rollback();
        }
        SQL_THROW(e.errcode(), "failed batch delete! {}", e.what());
    } catch (std::exception &e) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed batch delete! {}", e.what());
    } catch (...) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("failed batch delete! Unknown error!");
    }
}

inline void DBConnectBase::remove(const std::string &tablename, const std::string &where,
                                  bool autotrans) {
    if (autotrans) {
        transaction();
    }

    std::string sql = (where == "" || where == "1=1")
                        ? fmt::format("delete from {}", tablename, where)
                        : (fmt::format("delete from {} where {}", tablename, where));
    try {
        exec(sql);
        if (autotrans) {
            commit();
        }
    } catch (::hku::SQLException &e) {
        if (autotrans) {
            rollback();
        }
        SQL_THROW(e.errcode(), "Failed exec sql: {}! {}", sql, e.what());
    } catch (std::exception &e) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW("Failed exec sql: {}! {}", sql, e.what());
    } catch (...) {
        if (autotrans) {
            rollback();
        }
        HKU_THROW(R"(Failed exec sql: {}! Unknown error!)", sql);
    }
}

inline void DBConnectBase::remove(const std::string &tablename, const DBCondition &cond,
                                  bool autotrans) {
    remove(tablename, cond.str(), autotrans);
}

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_DBCONNECTBASE_H */