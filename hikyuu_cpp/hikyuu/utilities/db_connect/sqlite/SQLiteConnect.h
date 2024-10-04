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
 * @defgroup SQLite SQLite3 driver SQLITE3 数据驱动
 * @ingroup DBConnect
 */

/**
 * SQLite连接
 * @ingroup SQLite
 */
class HKU_UTILS_API SQLiteConnect : public DBConnectBase {
public:
    /**
     * 构造函数
     * @param param 数据库连接参数,支持如下参数：
     * <pre>
     * string db - 数据库文件名
     * int flags - SQLite连接方式：SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX
     *            （具体可参考 SQLite 帮助）
     * string key - sqlcipher 加密口令 (仅在使用 sqlcipher 引擎时生效)
     * </pre>
     */
    explicit SQLiteConnect(const Parameter &param);

    /** 析构函数 */
    virtual ~SQLiteConnect();

    /** 如果 sqlite 文件无效，返回false */
    virtual bool ping() override;

    virtual void transaction() override;
    virtual void commit() override;
    virtual void rollback() noexcept override;
    virtual int64_t exec(const std::string &sql_string) override;
    virtual SQLStatementPtr getStatement(const std::string &sql_statement) override;
    virtual bool tableExist(const std::string &tablename) override;
    virtual void resetAutoIncrement(const std::string &tablename) override;

    /**
     * @brief 对数据库进行检查
     * @note 该函数不能区分是因为文件并非sqlite文件，还是sqlite本身损坏的情况
     *       如有判断文件本身是否为有效sqlite文件，请使用 isValidSqliteFile 方法
     * @param quick true 快速检查 | false 完整性检查
     * @return true 检测成功
     * @return false 检测失败
     */
    bool check(bool quick = false) noexcept;

    /**
     * @brief 在线备份数据库，不影响其他数据库连接进行操作
     * @param zFilename 备份数据库名称
     * @param n_page 分批备份时每次循环备份的 page 数，小于等于0时，一次性备份，不进行分批备份
     * @param step_sleep 分批备份时每次循环后，休眠间隔时长（毫秒），以便让出cpu
     * @return true 成功
     * @return false 失败
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