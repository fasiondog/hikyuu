/*
 *  Copyright (c) 2019~2023 hikyuu.org
 *
 *  Created on: 2023/01/04
 *      Author: fasiondog
 */

#pragma once

#include "SQLiteConnect.h"

namespace hku {

/**
 * @brief SQLite 其他相关操作方法集合
 * @ingroup DBConnect
 */
class HKU_API SQLiteUtil {
public:
    SQLiteUtil() = default;
    ~SQLiteUtil() = default;

    enum BackupResult {
        BACKUP_SUCCESS = 0,             ///< 备份成功
        BACKUP_FAILED_INVALID_SRC = 1,  ///< 指定的待备份源数据无效
        BACKUP_FAILED_EXECUTE = 2,      ///< 执行备份出错
        BACKUP_FAILED_OTHER = 3,        ///< 其他原因导致备份失败原因
    };

    enum RecoverResult {
        RECOVER_SUCCESS = 0,              ///< 恢复成功
        RECOVER_FAILED_BACKUP_NOT_EXIST,  ///< 指定的备份文件不存在
        RECOVER_FAILED_BACKUP_INVALID,    ///< 指定的备份文件不是有效的数据库文件
        RECOVER_FAILED_INVALID_DST,  ///< 无效的指定的恢复目标文件名或文件被占用无法删除
        RECOVER_FAILED_EXECUTE,  ///< 执行恢复失败
        RECOVER_FAILED_OTHER,    ///< 其他原因导致的恢复失败
    };

    /**
     * @brief 在线备份数据库，不影响其他数据库连接进行操作
     * @param conn 待备份数据库连接
     * @param dst 备份数据库名称
     * @param n_page 分批备份时每次循环备份的 page 数，小于等于0时，一次性备份，不进行分批备份
     * @param step_sleep 分批备份时每次循环后，休眠间隔时长（毫秒），以便让出cpu
     */
    static BackupResult onlineBackup(const std::shared_ptr<SQLiteConnect>& conn,
                                     const std::string& dst, int n_page = -1,
                                     int step_sleep = 250) noexcept;
    /**
     * @brief 在线备份数据库，不影响其他数据库连接进行操作
     * @param src 待备份数据库名称
     * @param dst 备份数据库名称
     * @param n_page 分批备份时每次循环备份的 page 数，小于等于0时，一次性备份，不进行分批备份
     * @param step_sleep 分批备份时每次循环后，休眠间隔时长（毫秒），以便让出cpu
     */
    static BackupResult onlineBackup(const std::string& src, const std::string& dst,
                                     int n_page = -1, int step_sleep = 250) noexcept;

    /**
     * @brief 从指定的备份文件恢复数据库
     * @note
     * 待恢复的数据库文件在恢复过程中，可能先进行删除，如需保存损坏的数据库，请指定 save_bad 参数
     * @param backup 指定的备份文件
     * @param dst 待恢复的数据
     * @param save_bad 是否保存损坏的数据，将 dst 及 dst-journal 加上后缀 .bad 另存
     * @return RecoverResult
     */
    static RecoverResult recoverFromBackup(const std::string& backup, const std::string& dst,
                                           bool save_bad = false) noexcept;

    /**
     * @brief 尝试删除数据库相关文件（主文件及日志文件）
     * @param dbfilename 数据库文件名
     * @return true 指定的数据库文件及其日志文件都被成功删除或都不存在时，返回成功
     * @return false 指定的数据文件及其日志文件，其中一个删除失败都会返回删除失败
     */
    static bool removeDBFile(const std::string& dbfilename);
};

}  // namespace hku