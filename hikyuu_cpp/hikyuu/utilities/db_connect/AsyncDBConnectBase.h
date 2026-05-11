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
 * 异步数据库连接基类
 * @ingroup DBConnect
 *
 * 提供基于 boost::asio 协程的异步数据库操作接口。
 * 所有 I/O 密集型操作（如查询、执行）都返回 awaitable，可在协程中使用 co_await。
 *
 * @note 不是所有接口都需要改为 await 协程，仅 I/O 密集型操作需要异步化
 * @note 事务控制、配置访问等同步操作保持原有同步接口
 */
class HKU_UTILS_API AsyncDBConnectBase : public std::enable_shared_from_this<AsyncDBConnectBase> {
PARAMETER_SUPPORT  // NOSONAR

  public :
  /**
   * 构造函数
   * @param param 数据库连接参数
   */
  explicit AsyncDBConnectBase(const Parameter &param);
    virtual ~AsyncDBConnectBase() = default;

    //-------------------------------------------------------------------------
    // 子类接口 - 异步方法（返回 awaitable）
    //-------------------------------------------------------------------------

    /** ping 操作，用于判断是否连接 */
    virtual net::awaitable<bool> ping() = 0;

    /** 开始事务，失败时抛出异常 */
    virtual net::awaitable<void> transaction() = 0;

    /** 提交事务，失败时抛出异常 */
    virtual net::awaitable<void> commit() = 0;

    /** 回滚事务 */
    virtual net::awaitable<void> rollback() noexcept = 0;

    /** 执行无返回结果的 SQL */
    virtual net::awaitable<int64_t> exec(const std::string &sql_string) = 0;

    /** 获取 AsyncSQLStatement */
    virtual net::awaitable<AsyncSQLStatementPtr> getStatement(const std::string &sql_statement) = 0;

    /** 判断表是否存在 */
    virtual net::awaitable<bool> tableExist(const std::string &tablename) = 0;

    /**
     * 重置含自增 id 的表中的 id 从 1开始
     * @param tablename 待重置id的表名
     * @exception 表中仍旧含有数据时，抛出异常
     */
    virtual net::awaitable<void> resetAutoIncrement(const std::string &tablename) = 0;

    //-------------------------------------------------------------------------
    // 模板方法 - 异步版本
    //-------------------------------------------------------------------------

    /**
     * 保存或更新 通过 TABLE_BIND 绑定的表结构
     * @param item 待保持的记录
     * @param autotrans 启动事务
     */
    template <typename T>
    net::awaitable<void> save(T &item, bool autotrans = true);

    /**
     * 批量保存
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    net::awaitable<void> batchSave(Container &container, bool autotrans = true);

    /**
     * 批量保存，迭代器中的数据必须是通过 TABLE_BIND 绑定的表模型
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    net::awaitable<void> batchSave(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * 加载模型数据至指定的模型实例
     * @note 查询条件应只返回一条记录，如果有多条查询结果，将只取一条
     * @param item 指定的模型实例
     * @param where 查询条件，如："id=1"
     */
    template <typename T>
    net::awaitable<void> load(T &item, const std::string &where = "");

    /**
     * 加载模型数据至指定的模型实例
     * @note 查询条件应只返回一条记录，如果有多条查询结果，将只取一条
     * @param item 指定的模型实例
     * @param cond 查询条件
     */
    template <typename T>
    net::awaitable<void> load(T &item, const DBCondition &cond);

    /**
     * 加载模型数据至指定的模型实例, 仅供查询
     * @param item 指定的模型实例
     * @param sql 查询条件 select 的 sql 语句
     */
    template <typename T>
    net::awaitable<void> loadView(T &item, const std::string &sql);

    /**
     * 批量加载模型数据至容器
     * @param container 指定容器
     * @param where 查询条件
     */
    template <typename Container>
    net::awaitable<void> batchLoad(Container &container, const std::string &where = "");

    /**
     * 批量加载模型数据至容器
     * @param container 指定容器
     * @param cond 查询条件
     */
    template <typename Container>
    net::awaitable<void> batchLoad(Container &container, const DBCondition &cond);

    /**
     * 批量加载模型数据至容器
     * @param container 指定容器
     * @param sql select 的查询语句
     */
    template <typename Container>
    net::awaitable<void> batchLoadView(Container &container, const std::string &sql);

    /**
     * 批量更新
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    net::awaitable<void> batchUpdate(Container &container, bool autotrans = true);

    /**
     * 批量更新
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    net::awaitable<void> batchUpdate(InputIterator first, InputIterator last,
                                     bool autotrans = true);

    /**
     * 批量保存或更新
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    net::awaitable<void> batchSaveOrUpdate(Container &container, bool autotrans = true);

    /**
     * 批量保存或更新
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    net::awaitable<void> batchSaveOrUpdate(InputIterator first, InputIterator last,
                                           bool autotrans = true);

    /**
     * 从指定表中删除符合条件的数据
     * @param tablename 待删除数据的表名
     * @param where 删除条件
     * @param autotrans 启动事务
     */
    net::awaitable<void> remove(const std::string &tablename, const std::string &where,
                                bool autotrans = true);

    /**
     * 从指定表中删除符合条件的数据
     * @param tablename 待删除数据的表名
     * @param cond 删除条件
     * @param autotrans 启动事务
     */
    net::awaitable<void> remove(const std::string &tablename, const DBCondition &cond,
                                bool autotrans = true);

    /**
     * 删除
     * @param item 待删除的数据, 通过 item.rowid() 删除，删除后，rowid 将被置为无效
     * @param autotrans 启动事务
     */
    template <typename T>
    net::awaitable<void> remove(T &item, bool autotrans = true);

    /**
     * 批量删除
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    net::awaitable<void> batchRemove(Container &container, bool autotrans = true);

    /**
     * 批量删除，迭代器中的数据必须是通过 TABLE_BIND 绑定的表模型
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    net::awaitable<void> batchRemove(InputIterator first, InputIterator last,
                                     bool autotrans = true);

    /**
     * 查询单个整数，如：select count(*) from table
     * @note sql 语句应只返回单个元素，否则将抛出异常，如多条记录、多个列
     * @param query 查询语句
     * @param default_val 当查询失败时，返回该默认值。如果该值为 Null<int>(), 则抛出异常。
     */
    net::awaitable<int> queryInt(const std::string &query, int default_val);

    /**
     * 查询统计数据，如：select count(*) from table
     * @note sql 语句应只返回单个元素，否则将抛出异常，如多条记录、多个列
     * @param query 查询语句
     * @param default_val 当查询失败时，返回该默认值。如果该值为 Null<NumberType>(), 则抛出异常。
     */
    template <typename NumberType>
    net::awaitable<NumberType> queryNumber(const std::string &query,
                                           NumberType default_val = Null<NumberType>());

    /**
     * 分页查询
     * @tparam TableT 查询数据结构
     * @tparam page_size 每页数据记录数
     * @return AsyncSQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    AsyncSQLResultSet<TableT, page_size> query();

    /**
     * 分页查询
     * @tparam TableT 查询数据结构
     * @tparam page_size 每页数据记录数
     * @param query 查询条件
     * @return AsyncSQLResultSet<TableT, page_size>
     */
    template <typename TableT, size_t page_size = 50>
    AsyncSQLResultSet<TableT, page_size> query(const std::string &query);

    /**
     * 分页查询
     * @tparam TableT 查询数据结构
     * @tparam page_size 每页数据记录数
     * @param cond 查询条件
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
// inline方法实现
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
    st->getColumn(0, result);  // getColumn 是同步方法

    if (co_await st->moveNext()) {
        HKU_CHECK(default_val != Null<NumberType>(), "query doesn't result in exactly 1 element");
        co_return default_val;
    }

    co_return result;
}

//-------------------------------------------------------------------------
// 模板方法实现
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
            item.rowid(st->getLastRowid());  // getLastRowid 是同步方法
        }

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // 在 try-catch 外部处理回滚
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // 忽略回滚异常，保留原始异常
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
            iter->rowid(st->getLastRowid());  // getLastRowid 是同步方法
        }

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // 在 try-catch 外部处理回滚
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // 忽略回滚异常，保留原始异常
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

    // 在 try-catch 外部处理回滚
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // 忽略回滚异常，保留原始异常
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

    // 在 try-catch 外部处理回滚
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // 忽略回滚异常，保留原始异常
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
            co_await remove(*iter, false);  // 外层已处理事务
        }

        if (autotrans) {
            co_await commit();
        }
    } catch (...) {
        saved_exception = std::current_exception();
    }

    // 在 try-catch 外部处理回滚
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // 忽略回滚异常，保留原始异常
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

    // 在 try-catch 外部处理回滚
    if (saved_exception) {
        if (autotrans) {
            try {
                co_await rollback();
            } catch (...) {
                // 忽略回滚异常，保留原始异常
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
