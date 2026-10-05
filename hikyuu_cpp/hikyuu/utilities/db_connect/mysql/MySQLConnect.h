/*
 * MySQLConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#pragma once
#ifndef HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H
#define HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H

#include "../DBConnectBase.h"
#include "MySQLStatement.h"

#include <memory>
#include <cctype>

namespace hku {

namespace detail {

/**
 * Whether the SQL statement is read-only (SELECT/SHOW/DESC/DESCRIBE/EXPLAIN), i.e. safe to
 * replay after a reconnection. A write statement that fails with a lost connection may already
 * have been committed server-side, so replaying it would apply the write twice.
 */
inline bool isReadOnlySql(const std::string &sql) {
    size_t pos = sql.find_first_not_of(" \t\r\n(");
    if (pos == std::string::npos) {
        return false;
    }
    std::string word;
    while (pos < sql.size() && std::isalpha(static_cast<unsigned char>(sql[pos]))) {
        word += static_cast<char>(std::toupper(static_cast<unsigned char>(sql[pos])));
        ++pos;
    }
    return word == "SELECT" || word == "SHOW" || word == "DESC" || word == "DESCRIBE" ||
           word == "EXPLAIN";
}

/** Whether the MySQL error code indicates a lost connection
 * (CR_SERVER_GONE_ERROR/CR_SERVER_LOST) */
inline bool isConnectionLostError(int errcode) {
    return errcode == 2006 || errcode == 2013;
}

}  // namespace detail

class HKU_UTILS_API MySQLConnect : public DBConnectBase {
public:
    explicit MySQLConnect(const Parameter &param);
    virtual ~MySQLConnect() override;

    MySQLConnect(const MySQLConnect &) = delete;
    MySQLConnect &operator=(const MySQLConnect &) = delete;

    virtual bool ping() override;

    virtual int64_t exec(const std::string &sql_string) override;
    virtual SQLStatementPtr getStatement(const std::string &sql_statement) override;
    virtual bool tableExist(const std::string &tablename) override;
    virtual void resetAutoIncrement(const std::string &tablename) override;

    virtual void transaction() override;
    virtual void commit() override;
    virtual void rollback() noexcept override;

private:
    friend class MySQLStatement;

    // The method provided to MySQLStatement to access the original connection
    void *getRawConnection() const noexcept;

    // Internal helper methods
    bool tryConnect() noexcept;
    void connect();
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H */