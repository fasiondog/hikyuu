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
#include "../../DataType.h"

namespace hku {

class DBConnectBase;
typedef shared_ptr<DBConnectBase> DBConnectPtr;

class null_blob_exception: public exception {
public:
    null_blob_exception(): exception("Blob is null!") {}
};

/**
 * SQL Statement 基类
 * @ingroup DataDriver
 */
class HKU_API SQLStatementBase {
public:
    SQLStatementBase(const DBConnectPtr& driver, const string& sql_statement);

    virtual ~SQLStatementBase() = default;

    /** 获取构建时传入的表达式SQL语句 */
    const string& getSqlString() const;

    /** 获取数据驱动 */
    DBConnectPtr getDriver() const;

    /** 当前 SQL 表达式是否有效 */
    bool isValid() const;

    /** 执行 SQL */
    void exec();

    /** 移动至下一结果 */
    bool moveNext();

    void bind(int idx); //bind_null
    void bind(int idx, float item);
    void bind(int idx, double item);
    void bind(int idx, const string& item);
    void bindBlob(int idx, const string& item);

    template <typename T>
    typename std::enable_if<std::numeric_limits<T>::is_integer>::type
    bind(int idx, const T& item);

    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer>::type
    bind(int idx, const T& item);

    template <typename T, typename... Args>
    void bind(int idx, const T&, const Args&... rest); 

    int getNumColumns() const;

    //void getColumn(int idx, int64& item);
    void getColumn(int idx, double& item);
    void getColumn(int idx, float& item);
    void getColumn(int idx, string& item);

    template <typename T>
    typename std::enable_if<std::numeric_limits<T>::is_integer>::type
    getColumn(int idx, T&);

    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer>::type
    getColumn(int idx, T&);

    template <typename T, typename... Args>
    void getColumn(int idx, T&, Args&... rest); 

    //-------------------------------------------------------------------------
    // 子类接口
    //-------------------------------------------------------------------------
    virtual bool sub_isValid() const = 0;
    virtual void sub_exec() = 0;
    virtual bool sub_moveNext() = 0;

    virtual void sub_bindNull(int idx) = 0;
    virtual void sub_bindInt(int idx, int64 value) = 0;
    virtual void sub_bindDouble(int idx, double item) = 0;
    virtual void sub_bindText(int idx, const string& item) = 0;
    virtual void sub_bindBlob(int idx, const string& item) = 0;

    virtual int sub_getNumColumns() const = 0;
    virtual void sub_getColumnAsInt64(int idx, int64&) = 0;
    virtual void sub_getColumnAsDouble(int idx, double&) = 0;
    virtual void sub_getColumnAsText(int idx, string&) = 0;
    virtual void sub_getColumnAsBlob(int idx, string&) = 0;

private:
    SQLStatementBase() = delete;

protected:
    DBConnectPtr m_driver;
    string m_sql_string;
};

/** @ingroup DataDriver */
typedef shared_ptr<SQLStatementBase> SQLStatementPtr;


inline SQLStatementBase
::SQLStatementBase(const DBConnectPtr& driver, const string& sql_statement)
: m_driver(driver), m_sql_string(sql_statement) {
    HKU_ASSERT_M(driver, "driver is null!");
}

inline const string& SQLStatementBase::getSqlString() const {
    return m_sql_string;
}

inline DBConnectPtr SQLStatementBase::getDriver() const {
    return m_driver;
}

inline bool SQLStatementBase::isValid() const {
    return m_driver && sub_isValid() ? true : false;
}

inline void SQLStatementBase::bind(int idx, float item) {
    bind(idx, (double)item);
}

inline void SQLStatementBase::exec() {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_exec();
}

inline bool SQLStatementBase::moveNext() {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    return sub_moveNext();
}

inline void SQLStatementBase::bind(int idx) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_bindNull(idx);
}

inline void SQLStatementBase::bind(int idx, const string& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_bindText(idx, item);
}

inline void SQLStatementBase::bind(int idx, double item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_bindDouble(idx, item);
}

inline void SQLStatementBase::bindBlob(int idx, const string& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_bindBlob(idx, item);
}

inline int SQLStatementBase::getNumColumns() const {
    return sub_getNumColumns();
}

inline void SQLStatementBase::getColumn(int idx, double& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_getColumnAsDouble(idx, item);
}

inline void SQLStatementBase::getColumn(int idx, float& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    double temp;
    sub_getColumnAsDouble(idx, temp);
    item = (float)temp;
}

inline void SQLStatementBase::getColumn(int idx, string& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_getColumnAsText(idx, item);
}

template <typename T>
typename std::enable_if<std::numeric_limits<T>::is_integer>::type
SQLStatementBase::bind(int idx, const T& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    sub_bindInt(idx, item);
}

template <typename T>
typename std::enable_if<!std::numeric_limits<T>::is_integer>::type
SQLStatementBase::bind(int idx, const T& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    std::ostringstream sout;
    boost::archive::binary_oarchive oa(sout);
    oa << BOOST_SERIALIZATION_NVP(item);
    sub_bindBlob(idx, sout.str());
}

template <typename T>
typename std::enable_if<std::numeric_limits<T>::is_integer>::type
SQLStatementBase::getColumn(int idx, T& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    int64 temp;
    sub_getColumnAsInt64(idx, temp);
    item = (T)temp;
}

template <typename T>
typename std::enable_if<!std::numeric_limits<T>::is_integer>::type
SQLStatementBase::getColumn(int idx, T& item) {
    HKU_ASSERT_M(isValid(), "Invalid statement!");
    string tmp;
    try {
        sub_getColumnAsBlob(idx, tmp);
    } catch (null_blob_exception&) {
        return;
    }
    std::istringstream sin(tmp);
    boost::archive::binary_iarchive ia(sin);
    ia >> BOOST_SERIALIZATION_NVP(item); 
}

template <typename T, typename... Args>
void SQLStatementBase::bind(int idx, const T& item, const Args&... rest) {
    bind(idx, item);
    bind(idx+1, rest...);
}

template <typename T, typename... Args>
void SQLStatementBase::getColumn(int i, T& item, Args&... rest) {
    getColumn(i, item);
    getColumn(i+1, rest...);
}

} /* namespace */

#endif /* HIKYUU_DB_CONNECT_SQLSTATEMENTBASE_H */
