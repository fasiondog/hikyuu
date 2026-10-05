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
#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/Null.h"
#include "hikyuu/utilities/datetime/Datetime.h"
#include "hikyuu/utilities/exception.h"
#include "hikyuu/utilities/Log.h"
#include "../../DataType.h"
#include "DBCondition.h"
#include "SQLException.h"

namespace hku {

class DBConnectBase;

/** @ingroup DBConnect */
typedef std::shared_ptr<DBConnectBase> DBConnectPtr;

/** @ingroup DBConnect */
class null_blob_exception : public exception {
public:
    /** Construct */
    null_blob_exception() : exception("Blob is null!") {}
};

/**
 * Base class of the SQL statement
 * @ingroup DBConnect
 */
class HKU_UTILS_API SQLStatementBase {
public:
    /**
     * Constructor
     * @param driver database connection
     * @param sql_statement SQL statement
     */
    SQLStatementBase(DBConnectBase *driver, const std::string &sql_statement);

    virtual ~SQLStatementBase() = default;

    /** Get the expression SQL statement passed at construction */
    const std::string &getSqlString() const;

    /** Get the data driver */
    DBConnectBase *getConnect() const;

    /** Execute the SQL */
    void exec();

    /** Move to the next result */
    bool moveNext();

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
     * The values are bound at index 0, 1, ... so they line up with the ? written left to right by
     * renumberPlaceholders. It has to be the only binder of the statement: mixing it with the
     * index-based bind calls shifts every following value silently on the drivers that do not check
     * the binding order.
     */
    void bind_params(const BoundValues &params);

    /** Get the rowid of the last record inserted by the INSERT execution, it is not thread safe */
    uint64_t getLastRowid();

    /** Get the number of the table columns */
    int getNumColumns() const;

    /**
     * @name getColumn NULL mapping
     * A SQL NULL column read is mapped to the type's Null sentinel so it stays distinguishable
     * from real 0 values:
     * - signed integers (any width): Null<T>(), i.e. std::numeric_limits<T>::max()
     * - uint64_t: std::numeric_limits<uint64_t>::max()
     * - double/float: Null<double>() (quiet NaN)
     * - Datetime: Null<Datetime>()
     * - text: an empty string (text has no sentinel, NULL is indistinguishable from empty)
     * - blob: throws null_blob_exception; an empty (zero-length) blob returns an empty result
     *         instead, so NULL and empty blobs stay distinguishable
     *
     * @note An actually stored maximum value of an integer type is indistinguishable from NULL,
     *       which is inherent to the Null sentinel scheme
     */
    /** Get the data given by idx into item */
    void getColumn(int idx, double &item);

    /** Get the data given by idx into item */
    void getColumn(int idx, float &item);

    /** Get the data given by idx into item */
    void getColumn(int idx, std::string &item);

    /** Get the data given by idx into item */
    void getColumn(int idx, Datetime &item);

    void getColumn(int idx, std::vector<char> &);

    /** Get the data given by idx into item */
    template <typename T>
    typename std::enable_if<std::numeric_limits<T>::is_integer>::type getColumn(int idx, T &);

    /** Get the data given by idx into item */
    template <typename T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer>::type getColumn(int idx, T &);

    /** Get the given data into item1, item2 and item3 sequentially starting from the given idx */
    template <typename T, typename... Args>
    void getColumn(int idx, T &, Args &...rest);

    //-------------------------------------------------------------------------
    // Subclass interface
    //-------------------------------------------------------------------------
    virtual void sub_exec() = 0;              ///< Subclass interface @see exec
    virtual bool sub_moveNext() = 0;          ///< Subclass interface @see moveNext
    virtual uint64_t sub_getLastRowid() = 0;  ///< Subclass interface @see getLastRowid();

    virtual void sub_bindNull(int idx) = 0;                ///< Subclass interface @see bind
    virtual void sub_bindInt(int idx, int64_t value) = 0;  ///< Subclass interface @see bind
    virtual void sub_bindUInt64(int idx,
                                uint64_t value);  ///< Subclass interface @see bind. The default
                                                  ///< implementation rejects values above
                                                  ///< INT64_MAX and otherwise forwards to
                                                  ///< sub_bindInt; drivers with a native unsigned
                                                  ///< binding channel should override it
    virtual void sub_bindDouble(int idx, double item) = 0;  ///< Subclass interface @see bind
    virtual void sub_bindDatetime(int idx,
                                  const Datetime &item) = 0;  ///< Subclass interface @see bind
    virtual void sub_bindText(int idx,
                              const std::string &item) = 0;  ///< Subclass interface @see bind
    virtual void sub_bindText(int idx, const char *item, size_t len) = 0;  ///< Subclass interface
                                                                           ///< @see bind
    virtual void sub_bindBlob(int idx,
                              const std::string &item) = 0;  ///< Subclass interface @see bind
    virtual void sub_bindBlob(int idx, const std::vector<char> &item) = 0;  ///< Subclass interface
                                                                            ///< @see bind

    virtual int sub_getNumColumns() const = 0;  ///< Subclass interface @see getNumColumns
    virtual void sub_getColumnAsInt64(int idx,
                                      int64_t &) = 0;  ///< Subclass interface @see getColumn
    virtual void sub_getColumnAsUInt64(int idx,
                                       uint64_t &);  ///< Subclass interface @see getColumn. The
                                                     ///< default implementation forwards to
                                                     ///< sub_getColumnAsInt64 and rejects negative
                                                     ///< values; drivers that expose an unsigned
                                                     ///< column flag should override it
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
    SQLStatementBase() = delete;

protected:
    DBConnectBase *m_driver;   ///< Database connection
    std::string m_sql_string;  ///< Original SQL statement
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

inline void SQLStatementBase::sub_bindUInt64(int idx, uint64_t value) {
    SQL_CHECK(value <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()), -1,
              "The driver cannot bind uint64 value {} above INT64_MAX at index {}", value, idx);
    sub_bindInt(idx, static_cast<int64_t>(value));
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

inline void SQLStatementBase::sub_getColumnAsUInt64(int idx, uint64_t &item) {
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
    if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, unsigned long long> ||
                  (std::is_unsigned_v<T> && sizeof(T) == 8)) {
        sub_bindUInt64(idx, static_cast<uint64_t>(item));
    } else {
        sub_bindInt(idx, static_cast<int64_t>(item));
    }
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
        SQL_CHECK(temp >= static_cast<int64_t>(std::numeric_limits<T>::min()) &&
                    temp <= static_cast<int64_t>(std::numeric_limits<T>::max()),
                  -1, "Column {} value {} overflows {}", idx, temp, typeid(T).name());
        item = static_cast<T>(temp);
    } else if constexpr (std::is_unsigned_v<T>) {
        // Smaller unsigned types: reject negative values and values above the target range
        int64_t temp;
        sub_getColumnAsInt64(idx, temp);
        if (temp == Null<int64_t>()) {
            item = (std::numeric_limits<T>::max)();
            return;
        }
        SQL_CHECK(temp >= 0 && static_cast<uint64_t>(temp) <=
                                 static_cast<uint64_t>(std::numeric_limits<T>::max()),
                  -1, "Column {} value {} overflows {}", idx, temp, typeid(T).name());
        item = static_cast<T>(temp);
    } else {
        sub_getColumnAsInt64(idx, reinterpret_cast<int64_t &>(item));
    }
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

inline void SQLStatementBase::bind_params(const BoundValues &params) {
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

template <typename T, typename... Args>
void SQLStatementBase::getColumn(int idx, T &item, Args &...rest) {
    getColumn(idx, item);
    getColumn(idx + 1, rest...);
}

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_SQLSTATEMENTBASE_H */
