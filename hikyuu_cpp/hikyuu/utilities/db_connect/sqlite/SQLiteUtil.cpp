/*
 *  Copyright (c) 2023, hikyuu.org
 *
 *  Created on: 2023/01/04
 *      Author: fasiondog
 */

#include "hikyuu/utilities/os.h"
#include "SQLiteUtil.h"

namespace hku {

SQLiteUtil::BackupResult SQLiteUtil::onlineBackup(const std::shared_ptr<SQLiteConnect>& conn,
                                                  const std::string& dst, int n_page,
                                                  int step_sleep) noexcept {
    SPEND_TIME(SQLiteUtil_onlineBackup);
    HKU_IF_RETURN(!conn || !conn->ping(), BACKUP_FAILED_INVALID_SRC);
    return conn->backup(dst.c_str(), n_page, step_sleep) ? BACKUP_SUCCESS : BACKUP_FAILED_EXECUTE;
}

SQLiteUtil::BackupResult SQLiteUtil::onlineBackup(const std::string& src, const std::string& dst,
                                                  int n_page, int step_sleep) noexcept {
    try {
        Parameter param;
        param.set<std::string>("db", src);
        auto conn = std::make_shared<SQLiteConnect>(param);
        return onlineBackup(conn, dst, n_page, step_sleep);

    } catch (const std::exception& e) {
        HKU_WARN("Failed create SQLiteConnect! {}", e.what());
        return BACKUP_FAILED_OTHER;

    } catch (...) {
        HKU_WARN("Failed create SQLiteConnect! Unknow error!");
        return BACKUP_FAILED_OTHER;
    }
}

SQLiteUtil::RecoverResult SQLiteUtil::recoverFromBackup(const std::string& backup,
                                                        const std::string& dst,
                                                        bool save_bad) noexcept {
    SPEND_TIME(SQLiteUtil_recoverFromBackup);
    HKU_IF_RETURN(!existFile(backup), RECOVER_FAILED_BACKUP_NOT_EXIST);

    std::string journal = fmt::format("{}-journal", dst);
    if (save_bad) {
        // 如果目标数据库文件或日志文件存在，但另存为损坏文件备份时，失败返回
        HKU_IF_RETURN(
          existFile(journal) && !renameFile(journal, fmt::format("{}.bad", journal), true),
          RECOVER_FAILED_INVALID_DST);
        HKU_IF_RETURN(existFile(dst) && !renameFile(dst, fmt::format("{}.bad", dst), true),
                      RECOVER_FAILED_INVALID_DST);
    }

    // 尝试先删除目标数据库及其日志文件
    HKU_IF_RETURN(!removeDBFile(dst), RECOVER_FAILED_INVALID_DST);

    // 将备份数据库反向备份至目标数据库
    try {
        Parameter param;
        param.set<std::string>("db", backup);
        auto conn = std::make_shared<SQLiteConnect>(param);
        switch (onlineBackup(conn, dst, -1)) {
            case BACKUP_SUCCESS:
                return RECOVER_SUCCESS;
            case BACKUP_FAILED_INVALID_SRC:
                return RECOVER_FAILED_BACKUP_INVALID;
            case BACKUP_FAILED_EXECUTE:
                return RECOVER_FAILED_EXECUTE;
            case BACKUP_FAILED_OTHER:
                return RECOVER_FAILED_OTHER;
            default:
                return RECOVER_FAILED_OTHER;
        }

    } catch (const std::exception& e) {
        HKU_WARN("Failed create SQLiteConnect! {}", e.what());
        return RECOVER_FAILED_OTHER;

    } catch (...) {
        HKU_WARN("Failed create SQLiteConnect! Unknow error!");
        return RECOVER_FAILED_OTHER;
    }
}

bool SQLiteUtil::removeDBFile(const std::string& dbfilename) {
    bool success = true;
    std::string journal(fmt::format("{}-journal", dbfilename));
    if (existFile(journal) && !removeFile(journal)) {
        success = false;
    }

    if (existFile(dbfilename) && !removeFile(dbfilename)) {
        success = false;
    }

    return success;
}

}  // namespace hku