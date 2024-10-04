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
 * 数据库连接基类
 * @ingroup DBConnect
 */
class HKU_UTILS_API DBConnectBase : public std::enable_shared_from_this<DBConnectBase> {
    PARAMETER_SUPPORT  // NOSONAR

      public :
      /**
       * 构造函数
       * @param param 数据库连接参数
       */
      explicit DBConnectBase(const Parameter &param);
    virtual ~DBConnectBase() = default;

    //-------------------------------------------------------------------------
    // 子类接口
    //-------------------------------------------------------------------------

    /** ping 操作，用于判断是否连接 */
    virtual bool ping() = 0;

    /** 开始事务，失败时抛出异常 */
    virtual void transaction() = 0;

    /** 提交事务，失败时抛出异常 */
    virtual void commit() = 0;

    /** 回滚事务 */
    virtual void rollback() noexcept = 0;

    /** 执行无返回结果的 SQL */
    virtual int64_t exec(const std::string &sql_string) = 0;

    /** 获取 SQLStatement */
    virtual SQLStatementPtr getStatement(const std::string &sql_statement) = 0;

    /** 判断表是否存在 */
    virtual bool tableExist(const std::string &tablename) = 0;

    /**
     * 重置含自增 id 的表中的 id 从 1开始
     * @param tablename 待重置id的表名
     * @exception 表中仍旧含有数据时，抛出异常
     */
    virtual void resetAutoIncrement(const std::string &tablename) = 0;

    //-------------------------------------------------------------------------
    // 模板方法
    //-------------------------------------------------------------------------
    /**
     * 保存或更新 通过 TABLE_BIND 绑定的表结构
     * 可由 driver 直接保存，示例如下：
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
     * @param item 待保持的记录
     * @param autotrans 启动事务
     */
    template <typename T>
    void save(T &item, bool autotrans = true);

    /**
     * 批量保存
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    void batchSave(Container &container, bool autotrans = true);

    /**
     * 批量保存，迭代器中的数据必须是通过 TABLE_BIND 绑定的表模型
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    void batchSave(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * 加载模型数据至指定的模型实例
     * @note 查询条件应只返回一条记录，如果有多条查询结果，将只取一条
     * @param item 指定的模型实例
     * @param where 查询条件，如：“id=1"
     */
    template <typename T>
    void load(T &item, const std::string &where = "");

    /**
     * 加载模型数据至指定的模型实例
     * @note 查询条件应只返回一条记录，如果有多条查询结果，将只取一条
     * @param item 指定的模型实例
     * @param cond 查询条件，如：“id=1"
     */
    template <typename T>
    void load(T &item, const DBCondition &cond);

    /**
     * 加载模型数据至指定的模型实例, 仅供查询
     * @param item 指定的模型实例
     * @param sql 查询条件 select 的 sql 语句
     */
    template <typename T>
    void loadView(T &item, const std::string &sql);

    /**
     * 批量加载模型数据至容器（vector，list 等支持 push_back 的容器）
     * @param container 指定容器
     * @param where 查询条件
     */
    template <typename Container>
    void batchLoad(Container &container, const std::string &where = "");

    /**
     * 批量加载模型数据至容器（vector，list 等支持 push_back 的容器）
     * @param container 指定容器
     * @param cond 查询条件
     */
    template <typename Container>
    void batchLoad(Container &container, const DBCondition &cond);

    /**
     * 批量加载模型数据至容器（vector，list 等支持 push_back 的容器）
     * @param container 指定容器
     * @param sql select 的查询语句
     */
    template <typename Container>
    void batchLoadView(Container &container, const std::string &sql);

    /**
     * 批量更新
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    void batchUpdate(Container &container, bool autotrans = true);

    /**
     * 批量更新
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    void batchUpdate(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * 批量保存或更新
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    void batchSaveOrUpdate(Container &container, bool autotrans = true);

    /**
     * 批量保存或更新
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    void batchSaveOrUpdate(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * 从指定表中删除符合条件的数据指定条件删除
     * @param tablename 待删除数据的表名
     * @param where 删除条件
     * @param autotrans 启动事务
     */
    void remove(const std::string &tablename, const std::string &where, bool autotrans = true);

    /**
     * 从指定表中删除符合条件的数据指定条件删除
     * @param tablename 待删除数据的表名
     * @param cond 删除条件
     * @param autotrans 启动事务
     */
    void remove(const std::string &tablename, const DBCondition &cond, bool autotrans = true);

    /**
     * 删除
     * @param item 待删除的数据, 通过 item.rowid() 删除，删除后，rowid, 将被置为无效
     * @param autotrans 启动事务
     */
    template <typename T>
    void remove(T &item, bool autotrans = true);

    /**
     * 批量删除
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    void batchRemove(Container &container, bool autotrans = true);

    /**
     * 批量删除，迭代器中的数据必须是通过 TABLE_BIND 绑定的表模型
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    void batchRemove(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * 查询单个整数，如：select count(*) from table
     * @note sql 语句应只返回单个元素，否则将抛出异常，如多条记录、多个列
     * @param query 查询语句
     * @param default_val 当查询失败时，返回该默认值。如果该值为 Null<int>(), 则抛出异常。
     */
    int queryInt(const std::string &query, int default_val);

    /**
     * 查询统计数据，如：select count(*) from table
     * @note sql 语句应只返回单个元素，否则将抛出异常，如多条记录、多个列
     * @param query 查询语句
     * @param default_val 当查询失败时，返回该默认值。如果该值为 Null<NumberType>(), 则抛出异常。
     */
    template <typename NumberType>
    NumberType queryNumber(const std::string &query, NumberType default_val = Null<NumberType>());

    /**
     * 分页查询
     * @tparam TableT 查询数据结构
     * @tparam page_size 每页数据记录数
     * @return SQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    SQLResultSet<TableT, page_size> query();

    /**
     * 分页查询
     * @tparam TableT 查询数据结构
     * @tparam page_size 每页数据记录数
     * @param query 查询条件
     * @return SQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    SQLResultSet<TableT, page_size> query(const std::string &query);

    /**
     * 分页查询
     * @tparam TableT 查询数据结构
     * @tparam page_size 每页数据记录数
     * @param cond 查询条件
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
// inline方法实现
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
// 模板方法实现
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
    } catch (hku::SQLException &e) {
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
    } catch (hku::SQLException &e) {
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
    } catch (hku::SQLException &e) {
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
    } catch (hku::SQLException &e) {
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
    } catch (hku::SQLException &e) {
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
    } catch (hku::SQLException &e) {
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