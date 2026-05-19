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
 * SQLite 异步连接
 * @ingroup SQLite
 *
 * 基于 SQLite3 的异步数据库连接实现。
 * 每个连接内部持有独立的 ThreadPool(1) 单线程池，通过 co_run 将同步操作转换为异步接口。
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
     * @brief 对数据库进行检查（异步版本）
     * @note 该函数不能区分是因为文件并非sqlite文件，还是sqlite本身损坏的情况
     *       如有判断文件本身是否为有效sqlite文件，请使用 isValidSqliteFile 方法
     * @param quick true 快速检查 | false 完整性检查
     * @return true 检测成功
     * @return false 检测失败
     */
    net::awaitable<bool> check(bool quick = false);

    /**
     * @brief 在线备份数据库，不影响其他数据库连接进行操作（异步版本）
     * @param zFilename 备份数据库名称
     * @param n_page 分批备份时每次循环备份的 page 数，小于等于0时，一次性备份，不进行分批备份
     * @param step_sleep 分批备份时每次循环后，休眠间隔时长（毫秒），以便让出cpu
     * @return true 成功
     * @return false 失败
     */
    net::awaitable<bool> backup(const char *zFilename, int n_page = -1, int step_sleep = 250);

private:
    friend class AsyncSQLiteStatement;

    // 提供给 AsyncSQLiteStatement 访问原始连接的方法
    void *getRawConnection() const noexcept;

    // 提供给 AsyncSQLiteStatement 访问线程池执行器的方法
    ThreadPool::ExecutorWrapper getThreadPoolExecutor() const noexcept;

    // 内部辅助方法 - 异步版本
    net::awaitable<void> connect();

    // 内部辅助方法 - 同步版本（用于 Statement 构造时确保连接已初始化）
    void _connect();

    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_SQLITE_ASYNCSQLITECONNECT_H */
