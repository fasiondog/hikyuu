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

#include "../../DataType.h"
#include "../../utilities/Parameter.h"
#include "SQLStatementBase.h"

namespace hku {

/**
 * 数据库连接基类
 * @ingroup DBConnect
 */
class HKU_API DBConnectBase {
    PARAMETER_SUPPORT

public:
    /**
     * 构造函数
     * @param param 数据库连接参数
     */
    explicit DBConnectBase(const Parameter& param);
    virtual ~DBConnectBase() = default;

    //-------------------------------------------------------------------------
    // 子类接口
    //-------------------------------------------------------------------------
    /** 开始事务 */
    virtual void transaction() = 0;

    /** 提交事务 */
    virtual void commit() = 0;

    /** 回滚事务 */
    virtual void rollback() = 0;

    /** 检测连接是否可用 */
    virtual bool ping() = 0;

    /** 执行无返回结果的 SQL */
    virtual void exec(const string& sql_string) = 0;

    /** 获取 SQLStatement */
    virtual SQLStatementPtr getStatement(const string& sql_statement) = 0;

    /** 判断表是否存在 */
    virtual bool tableExist(const string& tablename) = 0;

    //-------------------------------------------------------------------------
    // 模板方法
    //-------------------------------------------------------------------------
    /**
     * 保存或更新 通过 TABLE_BIND 绑定的表结构
     * 可由 driver 直接保存，示例如下：
     * @code
     * class TTT {
     *   TABLE_BIND(TTT, ttt_table, age, name)
     *
     *   public:
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
    void save(T& item, bool autotrans = true);

    /**
     * 批量保存
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    void batchSave(Container& container, bool autotrans = true);

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
    void load(T& item, const string& where = "");

    /**
     * 批量加载模型数据至容器（vector，list 等支持 push_back 的容器）
     * @param container 指定容器
     * @param where 查询条件
     */
    template <typename Container>
    void batchLoad(Container& container, const string& where = "");

    /**
     * 批量更新
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    void batchUpdate(Container& container, bool autotrans = true);

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
    void batchSaveOrUpdate(Container& container, bool autotrans = true);

    /**
     * 批量保存或更新
     * @param first 迭代器起始点
     * @param last 迭代器终止点
     * @param autotrans 启动事务
     */
    template <class InputIterator>
    void batchSaveOrUpdate(InputIterator first, InputIterator last, bool autotrans = true);

    /**
     * 删除
     * @param item 待删除的数据
     * @param autotrans 启动事务
     */
    template <typename T>
    void remove(T& item, bool autotrans = true);

    /**
     * 批量删除
     * @param container 拥有迭代器的容器
     * @param autotrans 启动事务
     */
    template <class Container>
    void batchRemove(Container& container, bool autotrans = true);

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
     */
    int queryInt(const string& query);

private:
    DBConnectBase() = delete;
};

/** @ingroup DBConnect */
typedef shared_ptr<DBConnectBase> DBConnectPtr;

//-------------------------------------------------------------------------
// inline方法实现
//-------------------------------------------------------------------------

inline DBConnectBase::DBConnectBase(const Parameter& param) : m_params(param) {}

inline int DBConnectBase::queryInt(const string& query) {
    SQLStatementPtr st = getStatement(query);
    st->exec();
    HKU_CHECK((st->moveNext() && st->getNumColumns() == 1),
              "query doesn't result in exactly 1 element");
    int result = 0;
    st->getColumn(0, result);
    HKU_CHECK(!st->moveNext(), "query doesn't result in exactly 1 element");
    return result;
}

//-------------------------------------------------------------------------
// 模板方法实现
//-------------------------------------------------------------------------

template <typename T>
void DBConnectBase::save(T& item, bool autotrans) {
    SQLStatementPtr st =
      item.id() == 0 ? getStatement(T::getInsertSQL()) : getStatement(T::getUpdateSQL());
    if (autotrans) {
        transaction();
    }

    try {
        if (item.id() == 0) {
            item.save(st);
            st->exec();
            item.id(st->getLastRowid());
        } else {
            item.update(st);
            st->exec();
        }

        if (autotrans) {
            commit();
        }
    } catch (std::exception& e) {
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
inline void DBConnectBase::batchSave(Container& container, bool autotrans) {
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
            iter->id(st->getLastRowid());
        }

        if (autotrans) {
            commit();
        }

    } catch (std::exception& e) {
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
void DBConnectBase::load(T& item, const string& where) {
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

template <typename Container>
void DBConnectBase::batchLoad(Container& container, const string& where) {
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

template <class Container>
inline void DBConnectBase::batchUpdate(Container& container, bool autotrans) {
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

    } catch (std::exception& e) {
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

template <class InputIterator>
void DBConnectBase::batchSaveOrUpdate(InputIterator first, InputIterator last, bool autotrans) {
    vector<typename InputIterator::value_type> save_list;
    vector<typename InputIterator::value_type> update_list;
    for (auto iter = first; iter != last; ++iter) {
        if (iter->id() == 0) {
            save_list.push_back(*iter);
        } else {
            update_list.push_back(*iter);
        }
    }

    batchSave(save_list.begin(), save_list.end(), autotrans);
    batchUpdate(update_list.begin(), update_list.end(), autotrans);
}

template <class Container>
inline void DBConnectBase::batchSaveOrUpdate(Container& container, bool autotrans) {
    batchSaveOrUpdate(container.begin(), container.end(), autotrans);
}

template <typename T>
inline void DBConnectBase::remove(T& item, bool autotrans) {
    HKU_CHECK(item.id() != 0, "Invalid item, id is 0!");
    SQLStatementPtr st = getStatement(item.getDeleteSQL());
    if (autotrans) {
        transaction();
    }

    try {
        st->exec();
        if (autotrans) {
            commit();
        }
    } catch (std::exception& e) {
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
inline void DBConnectBase::batchRemove(Container& container, bool autotrans) {
    batchRemove(container.begin(), container.end(), autotrans);
}

template <class InputIterator>
inline void DBConnectBase::batchRemove(InputIterator first, InputIterator last, bool autotrans) {
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
    } catch (std::exception& e) {
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

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_DBCONNECTBASE_H */