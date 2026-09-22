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
 * @brief Collection of the other SQLite related operation methods
 * @ingroup DBConnect
 */
class HKU_UTILS_API SQLiteUtil {
public:
    SQLiteUtil() = default;
    ~SQLiteUtil() = default;

    enum BackupResult {
        BACKUP_SUCCESS = 0,             ///< Backup succeeded
        BACKUP_FAILED_INVALID_SRC = 1,  ///< The given source data to be backed up is invalid
        BACKUP_FAILED_EXECUTE = 2,      ///< An error occurred in the backup execution
        BACKUP_FAILED_OTHER = 3,        ///< The backup failed for other reasons
    };

    enum RecoverResult {
        RECOVER_SUCCESS = 0,              ///< Recovery succeeded
        RECOVER_FAILED_BACKUP_NOT_EXIST,  ///< The given backup file does not exist
        RECOVER_FAILED_BACKUP_INVALID,    ///< The given backup file is not a valid database file
        RECOVER_FAILED_INVALID_DST,  ///< The given recovery target file name is invalid or the file
                                     ///< is occupied and cannot be deleted
        RECOVER_FAILED_EXECUTE,      ///< The recovery execution failed
        RECOVER_FAILED_OTHER,        ///< The recovery failed for other reasons
    };

    /**
     * @brief Back up the database online, without affecting the operations of the other database
     * connections
     * @param conn the database connection to be backed up
     * @param dst the backup database name
     * @param n_page the number of the pages backed up in every loop of the batch backup; when it is
     *               less than or equal to 0 the backup is done in one go without batching
     * @param step_sleep the sleep interval in milliseconds after every loop of the batch backup, so
     *                   as to yield the CPU
     */
    static BackupResult onlineBackup(const std::shared_ptr<SQLiteConnect> &conn,
                                     const std::string &dst, int n_page = -1,
                                     int step_sleep = 250) noexcept;
    /**
     * @brief Back up the database online, without affecting the operations of the other database
     * connections
     * @param src the database name to be backed up
     * @param dst the backup database name
     * @param n_page the number of the pages backed up in every loop of the batch backup; when it is
     *               less than or equal to 0 the backup is done in one go without batching
     * @param step_sleep the sleep interval in milliseconds after every loop of the batch backup, so
     *                   as to yield the CPU
     */
    static BackupResult onlineBackup(const std::string &src, const std::string &dst,
                                     int n_page = -1, int step_sleep = 250) noexcept;

    /**
     * @brief Recover the database from the given backup file
     * @note
     * The database file to be recovered may be deleted first during the recovery; if the corrupted
     * database needs to be kept, please give the save_bad parameter
     * @param backup the given backup file
     * @param dst the data to be recovered
     * @param save_bad whether to keep the corrupted data; dst and dst-journal are saved separately
     *                 with the suffix .bad added
     * @return RecoverResult
     */
    static RecoverResult recoverFromBackup(const std::string &backup, const std::string &dst,
                                           bool save_bad = false) noexcept;

    /**
     * @brief Try to delete the database related files (the main file and the journal file)
     * @param dbfilename database file name
     * @return true success is returned when both the given database file and its journal file are
     *         deleted successfully or do not exist
     * @return false a deletion failure is returned if either the given database file or its journal
     *         file fails to be deleted
     */
    static bool removeDBFile(const std::string &dbfilename);
};

}  // namespace hku