/*
 * SQLStatemantBase.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_SQLSTATEMENTBASE_H
#define HIKYUU_DB_CONNECT_SQLSTATEMENTBASE_H

#include <type_traits>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "hikyuu/utilities/datetime/Datetime.h"
#include "../../DataType.h"

namespace hku {

class DBConnectBase;

/** @ingroup DBConnect */
typedef std::shared_ptr<DBConnectBase> DBConnectPtr;

/** @ingroup DBConnect */
class null_blob_exception : public exception {
public:
    /** 构造 */
    null_blob_exception() : exception("Blob is null!") {}
};

/**
 * SQL Statement 基类
 * @ingroup DBConnect
 */
class HKU_UTILS_API SQLStatementBase {
public:
    /**
     * 构造函数
     * @param driver 数据库连接
     * @param sql_statement SQL语句
     */
    SQLStatementBase(DBConnectBase *driver, const std::string &sql_statement);

    virtual ~SQLStatementBase() = default;

    /** 获取构建时传入的表达式SQL语句 */
    const std::string &getSqlString() const;

    /** 获取数据驱动 */
    DBConnectBase *getConnect() const;

    /** 执行 SQL */
    void exec();

    /** 移动至下一结果 */
    bool moveNext();

    /** 将 null 绑定至 idx 指定的 SQL 参数中 */
    void bind(int idx);  // bind_null

    /** 将 item 的值绑定至 idx 指定的 SQL 参数中 */
    void bind(int idx, float item);

    /** 将 item 的值绑定至 idx 指定的 SQL 参数中 */
    void bind(int idx, double item);

    /** 将 item 的值绑定至 idx 指定的 SQL 参数中 */
    void bind(int idx, const std::string &item);

    /** 将字符串类型 item 绑定至 idx 指定的 SQL 参数中 */
    void bind(int idx, const char *item, size_t len);

    /** 将 Datetime 类型 item 绑定至指定的 SQL 参数中 */
    void bind(int idx, const Datetime &item);

    /** 将 item 的值绑定至 idx 指定的 SQL 参数中 */
    void bindBlob(int idx, const std::string &item);

    /**
     * 将 item 的值绑定至 idx 指定的 SQL 参数中
     */
    void bindBlob(int idx, const std::vector<char> &time);

    /** 将 item 的值绑定至 idx 指定的 SQL 参数中 */
    template <typename T>
    typename std::enable_if<std::numeric_limits<T>::is_integer>::type bind(int idx, const T &item);

    /** 将 item 的值绑定至 idx 指定的 SQL 参数中 */
    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer>::type bind(int idx, const T &item);

    void bind(int idx, const std::vector<char> &item);

    /** 将 item 的值绑定至 idx 指定的 SQL 参数中 */
    template <typename T, typename... Args>
    void bind(int idx, const T &, const Args &...rest);

    /** 获取执行INSERT时最后插入记录的 rowid，非线程安全 */
    uint64_t getLastRowid();

    /** 获取表格列数 */
    int getNumColumns() const;

    /** 获取 idx 指定的数据至 item */
    void getColumn(int idx, double &item);

    /** 获取 idx 指定的数据至 item */
    void getColumn(int idx, float &item);

    /** 获取 idx 指定的数据至 item */
    void getColumn(int idx, std::string &item);

    /** 获取 idx 指定的数据至 item */
    void getColumn(int idx, Datetime &item);

    void getColumn(int idx, std::vector<char> &);

    /** 获取 idx 指定的数据至 item */
    template <typename T>
    typename std::enable_if<std::numeric_limits<T>::is_integer>::type getColumn(int idx, T &);

    /** 获取 idx 指定的数据至 item */
    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer>::type getColumn(int idx, T &);

    /** 以指定 idx 开始顺序获取指定的数据至 item1, item2, item3 */
    template <typename T, typename... Args>
    void getColumn(int idx, T &, Args &...rest);

    //-------------------------------------------------------------------------
    // 子类接口
    //-------------------------------------------------------------------------
    virtual void sub_exec() = 0;              ///< 子类接口 @see exec
    virtual bool sub_moveNext() = 0;          ///< 子类接口 @see moveNext
    virtual uint64_t sub_getLastRowid() = 0;  ///< 子类接口 @see getLastRowid();

    virtual void sub_bindNull(int idx) = 0;                            ///< 子类接口 @see bind
    virtual void sub_bindInt(int idx, int64_t value) = 0;              ///< 子类接口 @see bind
    virtual void sub_bindDouble(int idx, double item) = 0;             ///< 子类接口 @see bind
    virtual void sub_bindDatetime(int idx, const Datetime &item) = 0;  ///< 子类接口 @see bind
    virtual void sub_bindText(int idx, const std::string &item) = 0;   ///< 子类接口 @see bind
    virtual void sub_bindText(int idx, const char *item, size_t len) = 0;  ///< 子类接口 @see bind
    virtual void sub_bindBlob(int idx, const std::string &item) = 0;  ///< 子类接口 @see bind
    virtual void sub_bindBlob(int idx, const std::vector<char> &item) = 0;  ///< 子类接口 @see bind

    virtual int sub_getNumColumns() const = 0;                  ///< 子类接口 @see getNumColumns
    virtual void sub_getColumnAsInt64(int idx, int64_t &) = 0;  ///< 子类接口 @see getColumn
    virtual void sub_getColumnAsDouble(int idx, double &) = 0;  ///< 子类接口 @see getColumn
    virtual void sub_getColumnAsDatetime(int idx, Datetime &) = 0;  ///< 子类接口 @see getColumn
    virtual void sub_getColumnAsText(int idx, std::string &) = 0;   ///< 子类接口 @see getColumn
    virtual void sub_getColumnAsBlob(int idx, std::string &) = 0;   ///< 子类接口 @see getColumn
    virtual void sub_getColumnAsBlob(int idx,
                                     std::vector<char> &) = 0;  ///< 子类接口 @see getColumn

private:
    SQLStatementBase() = delete;

protected:
    DBConnectBase *m_driver;   ///< 数据库连接
    std::string m_sql_string;  ///< 原始 SQL 语句
};

/** @ingroup DBConnect */
typedef std::shared_ptr<SQLStatementBase> SQLStatementPtr;

inline SQLStatementBase ::SQLStatementBase(DBConnectBase *driver, const std::string &sql_statement)
: m_driver(driver), m_sql_string(sql_statement) {
    HKU_CHECK(driver, "driver is null!");
}

inline const std::string &SQLStatementBase::getSqlString() const {
    return m_sql_string;
}

inline DBConnectBase *SQLStatementBase::getConnect() const {
    return m_driver;
}

inline void SQLStatementBase::bind(int idx, float item) {
    bind(idx, (double)item);
}

inline void SQLStatementBase::exec() {
#if HKU_SQL_TRACE
    HKU_DEBUG(m_sql_string);
#endif
    sub_exec();
}

inline bool SQLStatementBase::moveNext() {
    return sub_moveNext();
}

inline void SQLStatementBase::bind(int idx) {
    sub_bindNull(idx);
}

inline void SQLStatementBase::bind(int idx, const std::string &item) {
    sub_bindText(idx, item);
}

inline void SQLStatementBase::bind(int idx, double item) {
    sub_bindDouble(idx, item);
}

inline void SQLStatementBase::bind(int idx, const Datetime &item) {
    sub_bindDatetime(idx, item);
}

inline void SQLStatementBase::bindBlob(int idx, const std::string &item) {
    sub_bindBlob(idx, item);
}

inline void SQLStatementBase::bindBlob(int idx, const std::vector<char> &item) {
    sub_bindBlob(idx, item);
}

inline uint64_t SQLStatementBase::getLastRowid() {
    return sub_getLastRowid();
}

inline int SQLStatementBase::getNumColumns() const {
    return sub_getNumColumns();
}

inline void SQLStatementBase::getColumn(int idx, double &item) {
    sub_getColumnAsDouble(idx, item);
}

inline void SQLStatementBase::getColumn(int idx, float &item) {
    double temp;
    sub_getColumnAsDouble(idx, temp);
    item = (float)temp;
}

inline void SQLStatementBase::getColumn(int idx, Datetime &item) {
    sub_getColumnAsDatetime(idx, item);
}

inline void SQLStatementBase::getColumn(int idx, std::string &item) {
    sub_getColumnAsText(idx, item);
}

inline void SQLStatementBase::bind(int idx, const std::vector<char> &item) {
    sub_bindBlob(idx, item);
}

template <typename T>
typename std::enable_if<std::numeric_limits<T>::is_integer>::type SQLStatementBase::bind(
  int idx, const T &item) {
    sub_bindInt(idx, item);
}

template <typename T>
typename std::enable_if<!std::numeric_limits<T>::is_integer>::type SQLStatementBase::bind(
  int idx, const T &item) {
    std::ostringstream sout;
    boost::archive::binary_oarchive oa(sout);
    oa << BOOST_SERIALIZATION_NVP(item);
    sub_bindBlob(idx, sout.str());
}

template <typename T>
typename std::enable_if<std::numeric_limits<T>::is_integer>::type SQLStatementBase::getColumn(
  int idx, T &item) {
    int64_t temp;
    sub_getColumnAsInt64(idx, temp);
    item = (T)temp;
}

inline void SQLStatementBase::getColumn(int idx, std::vector<char> &item) {
    sub_getColumnAsBlob(idx, item);
}

template <typename T>
typename std::enable_if<!std::numeric_limits<T>::is_integer>::type SQLStatementBase::getColumn(
  int idx, T &item) {
    std::string tmp;
    try {
        sub_getColumnAsBlob(idx, tmp);
    } catch (null_blob_exception &) {
        return;
    }
    std::istringstream sin(tmp);
    boost::archive::binary_iarchive ia(sin);
    ia >> BOOST_SERIALIZATION_NVP(item);
}

template <typename T, typename... Args>
void SQLStatementBase::bind(int idx, const T &item, const Args &...rest) {
    bind(idx, item);
    bind(idx + 1, rest...);
}

template <typename T, typename... Args>
void SQLStatementBase::getColumn(int idx, T &item, Args &...rest) {
    getColumn(idx, item);
    getColumn(idx + 1, rest...);
}

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_SQLSTATEMENTBASE_H */
