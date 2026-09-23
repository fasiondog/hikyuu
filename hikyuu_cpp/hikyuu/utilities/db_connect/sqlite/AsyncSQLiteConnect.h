/*
 * AsyncSQLiteConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */
#pragma once
#ifndef HIYUU_DB_CONNECT_SQLITE_ASYNCSQLITECONNECT_H
#define HIYUU_DB_CONNECT_SQLITE_ASYNCSQLITECONNECT_H

#include "../AsyncDBConnectBase.h"
#include "AsyncSQLiteStatement.h"
#include "../../thread/ThreadPool.h"

#include <memory>

namespace hku {

/**
 * Asynchronous SQLite connection
 * @ingroup SQLite
 *
 * The asynchronous database connection implementation based on SQLite3.
 * Every connection holds an independent ThreadPool(1) single thread pool internally, and the
 * synchronous operations are converted into the asynchronous interfaces through co_run.
 */
class HKU_UTILS_API AsyncSQLiteConnect : public AsyncDBConnectBase {
public:
    explicit AsyncSQLiteConnect(const Parameter &param);
    virtual ~AsyncSQLiteConnect() override;

    AsyncSQLiteConnect(const AsyncSQLiteConnect &) = delete;
    AsyncSQLiteConnect &operator=(const AsyncSQLiteConnect &) = delete;

    virtual net::awaitable<bool> ping() override;
    virtual net::awaitable<int64_t> exec(const std::string &sql_string) override;
    virtual net::awaitable<AsyncSQLStatementPtr> getStatement(
      const std::string &sql_statement) override;
    virtual net::awaitable<bool> tableExist(const std::string &tablename) override;
    virtual net::awaitable<void> resetAutoIncrement(const std::string &tablename) override;

    virtual net::awaitable<void> transaction() override;
    virtual net::awaitable<void> commit() override;
    virtual net::awaitable<void> rollback() noexcept override;

    /**
     * @brief Check the database (the asynchronous version)
     * @note This function cannot distinguish whether the file is not a sqlite file or the sqlite
     *       itself is corrupted
     *       If the file itself needs to be judged as a valid sqlite file, please use the
     *       isValidSqliteFile method
     * @param quick true quick check | false integrity check
     * @return true the check succeeded
     * @return false the check failed
     */
    net::awaitable<bool> check(bool quick = false);

    /**
     * @brief Back up the database online, without affecting the operations of the other database
     * connections (the asynchronous version)
     * @param zFilename the backup database name
     * @param n_page the number of the pages backed up in every loop of the batch backup; when it is
     *               less than or equal to 0 the backup is done in one go without batching
     * @param step_sleep the sleep interval in milliseconds after every loop of the batch backup, so
     *                   as to yield the CPU
     * @return true success
     * @return false failure
     */
    net::awaitable<bool> backup(const char *zFilename, int n_page = -1, int step_sleep = 250);

private:
    friend class AsyncSQLiteStatement;

    // The method provided to AsyncSQLiteStatement to access the original connection
    void *getRawConnection() const noexcept;

    // The method provided to AsyncSQLiteStatement to access the thread pool executor
    ThreadPool::ExecutorWrapper getThreadPoolExecutor() const noexcept;

    // Internal helper methods - the asynchronous version
    net::awaitable<void> connect();

    // Internal helper methods - the synchronous version (used to ensure the connection has been
    // initialized when the Statement is constructed)
    void _connect();

    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_SQLITE_ASYNCSQLITECONNECT_H */
