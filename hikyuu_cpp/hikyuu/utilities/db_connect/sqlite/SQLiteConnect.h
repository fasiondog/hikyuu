/*
 * SQLiteConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-1
 *      Author: fasiondog
 */
#pragma once
#ifndef HIYUU_DB_CONNECT_SQLITE_SQLITECONNECT_H
#define HIYUU_DB_CONNECT_SQLITE_SQLITECONNECT_H

#include <sqlite3.h>
#include "../DBConnectBase.h"
#include "SQLiteStatement.h"

namespace hku {

/**
 * @defgroup SQLite SQLite3 driver, the SQLITE3 data driver
 * @ingroup DBConnect
 */

/**
 * SQLite connection
 * @ingroup SQLite
 */
class HKU_UTILS_API SQLiteConnect : public DBConnectBase {
public:
    /**
     * Constructor
     * @param param database connection parameters, the following parameters are supported:
     * <pre>
     * string db - database file name
     * int flags - the SQLite connection way: SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
     *             SQLITE_OPEN_NOMUTEX
     *             (refer to the SQLite help for details)
     * string key - the sqlcipher encryption password (it takes effect when the sqlcipher engine is
     *              used only)
     * </pre>
     */
    explicit SQLiteConnect(const Parameter &param);

    /** Destructor */
    virtual ~SQLiteConnect() override;

    /** false is returned if the sqlite file is invalid */
    virtual bool ping() override;

    virtual void transaction() override;
    virtual void commit() override;
    virtual void rollback() noexcept override;
    virtual int64_t exec(const std::string &sql_string) override;
    virtual SQLStatementPtr getStatement(const std::string &sql_statement) override;
    virtual bool tableExist(const std::string &tablename) override;
    virtual void resetAutoIncrement(const std::string &tablename) override;

    /**
     * @brief Check the database
     * @note This function cannot distinguish whether the file is not a sqlite file or the sqlite
     *       itself is corrupted
     *       If the file itself needs to be judged as a valid sqlite file, please use the
     *       isValidSqliteFile method
     * @param quick true quick check | false integrity check
     * @return true the check succeeded
     * @return false the check failed
     */
    bool check(bool quick = false) noexcept;

    /**
     * @brief Back up the database online, without affecting the operations of the other database
     * connections
     * @param zFilename the backup database name
     * @param n_page the number of the pages backed up in every loop of the batch backup; when it is
     *               less than or equal to 0 the backup is done in one go without batching
     * @param step_sleep the sleep interval in milliseconds after every loop of the batch backup, so
     *                   as to yield the CPU
     * @return true success
     * @return false failure
     */
    bool backup(const char *zFilename, int n_page = -1, int step_sleep = 250) noexcept;

private:
    void close();

private:
    friend class SQLiteStatement;
    std::string m_dbname;
    sqlite3 *m_db;
};

typedef std::shared_ptr<SQLiteConnect> SQLiteConnectPtr;

}  // namespace hku

#endif /* HIYUU_DB_MANAGER_SQLITE_SQLITECONNECT_H */