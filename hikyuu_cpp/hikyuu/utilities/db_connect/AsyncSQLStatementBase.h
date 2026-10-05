/*
 * AsyncSQLStatementBase.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-07
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_ASYNCSQLSTATEMENTBASE_H
#define HIKYUU_DB_CONNECT_ASYNCSQLSTATEMENTBASE_H

#include <type_traits>
#include <sstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/datetime/Datetime.h"
#include "hikyuu/utilities/exception.h"
#include "hikyuu/utilities/Log.h"
#include "../net.h"
#include "SQLStatementBase.h"

namespace hku {

class AsyncDBConnectBase;

/** @ingroup DBConnect */
typedef std::shared_ptr<AsyncDBConnectBase> AsyncDBConnectPtr;

/**
 * Base class of the asynchronous SQL statement
 * @ingroup DBConnect
 *
 * It provides an asynchronous SQL statement execution interface based on the boost::asio coroutine.
 * All the I/O operations (such as exec, moveNext and getColumn) return an awaitable.
 *
 * @note It keeps the same bind and getColumn interface design as SQLStatementBase
 * @note Only the I/O intensive operations are made asynchronous; the local operations such as the
 *       data binding stay synchronous
 */
class HKU_UTILS_API AsyncSQLStatementBase {
public:
    /**
     * Constructor
     * @param driver database connection
     * @param sql_statement SQL statement
     */
    AsyncSQLStatementBase(AsyncDBConnectBase *driver, const std::string &sql_statement);

    virtual ~AsyncSQLStatementBase() = default;

    /** Get the expression SQL statement passed at construction */
    const std::string &getSqlString() const;

    /** Get the data driver */
    AsyncDBConnectBase *getConnect() const;

    //-------------------------------------------------------------------------
    // Asynchronous methods - they return an awaitable
    //-------------------------------------------------------------------------

    /** Execute the SQL */
    net::awaitable<void> exec();

    /** Move to the next result */
    net::awaitable<bool> moveNext();

    //-------------------------------------------------------------------------
    // Synchronous methods - the bind, getColumn and getLastRowid operations are local memory
    // operations, they stay synchronous
    //-------------------------------------------------------------------------

    /** Get the rowid of the last record inserted by the INSERT execution, it is not thread safe */
    uint64_t getLastRowid();

    /** Get the data given by idx into item.
     * A SQL NULL column is mapped to the type's Null sentinel, @see SQLStatementBase::getColumn */
    void getColumn(int idx, double &item);

    /** Get the data given by idx into item */
    void getColumn(int idx, float &item);

    /** Get the data given by idx into item */
    void getColumn(int idx, std::string &item);

    /** Get the data given by idx into item */
    void getColumn(int idx, Datetime &item);

    void getColumn(int idx, std::vector<char> &item);

    /** Get the data given by idx into item */
    template <typename T>
    typename std::enable_if<std::numeric_limits<T>::is_integer>::type getColumn(int idx, T &item);

    /** Get the data given by idx into item */
    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer>::type getColumn(int idx, T &item);

    /** Get the given data into item1, item2 and item3 sequentially starting from the given idx */
    template <typename T, typename... Args>
    void getColumn(int idx, T &item, Args &...rest);

    //-------------------------------------------------------------------------
    // Synchronous methods - they stay unchanged
    //-------------------------------------------------------------------------

    /** Bind null to the SQL parameter given by idx */
    void bind(int idx);  // bind_null

    /** Bind the value of item to the SQL parameter given by idx */
    void bind(int idx, float item);

    /** Bind the value of item to the SQL parameter given by idx */
    void bind(int idx, double item);

    /** Bind the value of item to the SQL parameter given by idx */
    void bind(int idx, const std::string &item);

    /** Bind the string type item to the SQL parameter given by idx */
    void bind(int idx, const char *item, size_t len);

    /** Bind the Datetime type item to the given SQL parameter */
    void bind(int idx, const Datetime &item);

    /** Bind the value of item to the SQL parameter given by idx */
    void bindBlob(int idx, const std::string &item);

    /**
     * Bind the value of item to the SQL parameter given by idx
     */
    void bindBlob(int idx, const std::vector<char> &time);

    /** Bind the value of item to the SQL parameter given by idx */
    template <typename T>
    typename std::enable_if<std::numeric_limits<T>::is_integer>::type bind(int idx, const T &item);

    /** Bind the value of item to the SQL parameter given by idx */
    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer>::type bind(int idx, const T &item);

    void bind(int idx, const std::vector<char> &item);

    /** Bind the value of item to the SQL parameter given by idx */
    template <typename T, typename... Args>
    void bind(int idx, const T &, const Args &...rest);

    /**
     * Bind a full set of condition values to the anonymous placeholders of the statement, in order
     *
     * It stays synchronous, like the other bind operations, and has to be the only binder of the
     * statement. @see SQLStatementBase::bind_params
     */
    void bind_params(const BoundValues &params);

    /** Get the number of the table columns */
    int getNumColumns() const;

    //-------------------------------------------------------------------------
    // Subclass interface - the asynchronous methods
    //-------------------------------------------------------------------------
    virtual net::awaitable<void> sub_exec() = 0;      ///< Subclass interface @see exec
    virtual net::awaitable<bool> sub_moveNext() = 0;  ///< Subclass interface @see moveNext

    //-------------------------------------------------------------------------
    // Subclass interface - the synchronous methods (the bind, getColumn and getLastRowid operations
    // are usually local, so they stay synchronous)
    //-------------------------------------------------------------------------
    virtual uint64_t sub_getLastRowid() = 0;               ///< Subclass interface @see getLastRowid
    virtual void sub_bindNull(int idx) = 0;                ///< Subclass interface @see bind
    virtual void sub_bindInt(int idx, int64_t value) = 0;  ///< Subclass interface @see bind
    virtual void sub_bindUInt64(int idx, uint64_t value) {
        if (value > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            throw exception("The driver cannot bind uint64 value above INT64_MAX");
        }
        sub_bindInt(idx, static_cast<int64_t>(value));
    }  ///< Subclass interface @see bind. Drivers with a native unsigned binding channel should
       ///< override it
    virtual void sub_bindDouble(int idx, double item) = 0;  ///< Subclass interface @see bind
    virtual void sub_bindDatetime(int idx, const Datetime &item) = 0;       ///< Subclass interface
                                                                            ///< @see bind
    virtual void sub_bindText(int idx, const std::string &item) = 0;        ///< Subclass interface
                                                                            ///< @see bind
    virtual void sub_bindText(int idx, const char *item, size_t len) = 0;   ///< Subclass interface
                                                                            ///< @see bind
    virtual void sub_bindBlob(int idx, const std::string &item) = 0;        ///< Subclass interface
                                                                            ///< @see bind
    virtual void sub_bindBlob(int idx, const std::vector<char> &item) = 0;  ///< Subclass interface
                                                                            ///< @see bind

    virtual int sub_getNumColumns() const = 0;  ///< Subclass interface
                                                ///< @see getNumColumns
    virtual void sub_getColumnAsInt64(int idx,
                                      int64_t &) = 0;  ///< Subclass interface @see getColumn
    virtual void sub_getColumnAsUInt64(int idx, uint64_t &item) {
        int64_t temp;
        sub_getColumnAsInt64(idx, temp);
        if (temp == Null<int64_t>()) {
            // A NULL column read back through the signed channel maps to the uint64 Null sentinel
            item = (std::numeric_limits<uint64_t>::max)();
            return;
        }
        // A negative int64 is the two's complement image of a uint64 bit pattern (drivers like
        // SQLite store every integer as int64), so reinterpret the bits instead of rejecting
        item = static_cast<uint64_t>(temp);
    }  ///< Subclass interface @see getColumn. Drivers that expose an unsigned column flag should
       ///< override it
    virtual void sub_getColumnAsDouble(int idx,
                                       double &) = 0;  ///< Subclass interface @see getColumn
    virtual void sub_getColumnAsDatetime(int idx,
                                         Datetime &) = 0;  ///< Subclass interface @see getColumn
    virtual void sub_getColumnAsText(int idx,
                                     std::string &) = 0;  ///< Subclass interface @see getColumn
    virtual void sub_getColumnAsBlob(int idx,
                                     std::string &) = 0;  ///< Subclass interface @see getColumn
    virtual void sub_getColumnAsBlob(
      int idx,
      std::vector<char> &) = 0;  ///< Subclass interface @see getColumn

private:
    AsyncSQLStatementBase() = delete;

protected:
    AsyncDBConnectBase *m_driver;  ///< Database connection
    std::string m_sql_string;      ///< Original SQL statement
};

/** @ingroup DBConnect */
typedef std::shared_ptr<AsyncSQLStatementBase> AsyncSQLStatementPtr;

inline AsyncSQLStatementBase::AsyncSQLStatementBase(AsyncDBConnectBase *driver,
                                                    const std::string &sql_statement)
: m_driver(driver), m_sql_string(sql_statement) {
    HKU_CHECK(driver, "driver is null!");
}

inline const std::string &AsyncSQLStatementBase::getSqlString() const {
    return m_sql_string;
}

inline AsyncDBConnectBase *AsyncSQLStatementBase::getConnect() const {
    return m_driver;
}

//-------------------------------------------------------------------------
// Implementation of the asynchronous methods
//-------------------------------------------------------------------------

inline net::awaitable<void> AsyncSQLStatementBase::exec() {
#if HKU_SQL_TRACE
    HKU_DEBUG(m_sql_string);
#endif
    co_await sub_exec();
}

inline net::awaitable<bool> AsyncSQLStatementBase::moveNext() {
    co_return co_await sub_moveNext();
}

inline uint64_t AsyncSQLStatementBase::getLastRowid() {
    return sub_getLastRowid();
}

//-------------------------------------------------------------------------
// Implementation of the synchronous methods (the bind and getColumn operations are local memory
// operations, they stay synchronous)
//-------------------------------------------------------------------------

inline void AsyncSQLStatementBase::getColumn(int idx, double &item) {
    sub_getColumnAsDouble(idx, item);
}

inline void AsyncSQLStatementBase::getColumn(int idx, float &item) {
    double temp;
    sub_getColumnAsDouble(idx, temp);
    item = (float)temp;
}

inline void AsyncSQLStatementBase::getColumn(int idx, Datetime &item) {
    sub_getColumnAsDatetime(idx, item);
}

inline void AsyncSQLStatementBase::getColumn(int idx, std::string &item) {
    sub_getColumnAsText(idx, item);
}

inline void AsyncSQLStatementBase::getColumn(int idx, std::vector<char> &item) {
    sub_getColumnAsBlob(idx, item);
}

template <typename T>
typename std::enable_if<std::numeric_limits<T>::is_integer>::type AsyncSQLStatementBase::getColumn(
  int idx, T &item) {
    if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, unsigned long long> ||
                  (std::is_unsigned_v<T> && sizeof(T) == 8)) {
        uint64_t temp;
        sub_getColumnAsUInt64(idx, temp);
        item = static_cast<T>(temp);
    } else if constexpr (std::is_signed_v<T> && sizeof(T) < 8) {
        // Reject silently narrowing an int64 value into a smaller signed type
        int64_t temp;
        sub_getColumnAsInt64(idx, temp);
        if (temp == Null<int64_t>()) {
            // Map the NULL sentinel onto the target type's own Null sentinel
            item = (std::numeric_limits<T>::max)();
            return;
        }
        if (temp < static_cast<int64_t>(std::numeric_limits<T>::min()) ||
            temp > static_cast<int64_t>(std::numeric_limits<T>::max())) {
            throw exception("Column value overflows the target integer type");
        }
        item = static_cast<T>(temp);
    } else if constexpr (std::is_unsigned_v<T>) {
        // Smaller unsigned types: reject negative values and values above the target range
        int64_t temp;
        sub_getColumnAsInt64(idx, temp);
        if (temp == Null<int64_t>()) {
            item = (std::numeric_limits<T>::max)();
            return;
        }
        if (temp < 0 ||
            static_cast<uint64_t>(temp) > static_cast<uint64_t>(std::numeric_limits<T>::max())) {
            throw exception("Column value overflows the target unsigned integer type");
        }
        item = static_cast<T>(temp);
    } else {
        sub_getColumnAsInt64(idx, reinterpret_cast<int64_t &>(item));
    }
}

template <typename T>
typename std::enable_if<!std::numeric_limits<T>::is_integer>::type AsyncSQLStatementBase::getColumn(
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
void AsyncSQLStatementBase::getColumn(int idx, T &item, Args &...rest) {
    getColumn(idx, item);
    getColumn(idx + 1, rest...);
}

//-------------------------------------------------------------------------
// Implementation of the synchronous methods (the bind operations stay synchronous)
//-------------------------------------------------------------------------

inline void AsyncSQLStatementBase::bind(int idx, float item) {
    bind(idx, (double)item);
}

inline void AsyncSQLStatementBase::bind(int idx) {
    sub_bindNull(idx);
}

inline void AsyncSQLStatementBase::bind(int idx, const std::string &item) {
    sub_bindText(idx, item);
}

inline void AsyncSQLStatementBase::bind(int idx, double item) {
    sub_bindDouble(idx, item);
}

inline void AsyncSQLStatementBase::bind(int idx, const Datetime &item) {
    sub_bindDatetime(idx, item);
}

inline void AsyncSQLStatementBase::bindBlob(int idx, const std::string &item) {
    sub_bindBlob(idx, item);
}

inline void AsyncSQLStatementBase::bindBlob(int idx, const std::vector<char> &item) {
    sub_bindBlob(idx, item);
}

inline int AsyncSQLStatementBase::getNumColumns() const {
    return sub_getNumColumns();
}

inline void AsyncSQLStatementBase::bind(int idx, const std::vector<char> &item) {
    sub_bindBlob(idx, item);
}

template <typename T>
typename std::enable_if<std::numeric_limits<T>::is_integer>::type AsyncSQLStatementBase::bind(
  int idx, const T &item) {
    if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, unsigned long long> ||
                  (std::is_unsigned_v<T> && sizeof(T) == 8)) {
        sub_bindUInt64(idx, static_cast<uint64_t>(item));
    } else {
        sub_bindInt(idx, static_cast<int64_t>(item));
    }
}

template <typename T>
typename std::enable_if<!std::numeric_limits<T>::is_integer>::type AsyncSQLStatementBase::bind(
  int idx, const T &item) {
    std::ostringstream sout;
    boost::archive::binary_oarchive oa(sout);
    oa << BOOST_SERIALIZATION_NVP(item);
    sub_bindBlob(idx, sout.str());
}

template <typename T, typename... Args>
void AsyncSQLStatementBase::bind(int idx, const T &item, const Args &...rest) {
    bind(idx, item);
    bind(idx + 1, rest...);
}

inline void AsyncSQLStatementBase::bind_params(const BoundValues &params) {
    for (size_t i = 0, len = params.size(); i < len; ++i) {
        std::visit(
          [&](const auto &value) {
              using U = std::decay_t<decltype(value)>;
              if constexpr (std::is_same_v<U, std::nullptr_t>) {
                  bind(static_cast<int>(i));
              } else {
                  bind(static_cast<int>(i), value);
              }
          },
          params[i]);
    }
}

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_ASYNCSQLSTATEMENTBASE_H */
