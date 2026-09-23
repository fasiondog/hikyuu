/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#include "DBUpgrade.h"

#include "hikyuu/utilities/config.h"
#if HKU_ENABLE_MYSQL
#include "mysql/MySQLConnect.h"
#include "mysql/AsyncMySQLConnect.h"
#endif

#if HKU_ENABLE_SQLITE
#include "sqlite/SQLiteConnect.h"
#include "sqlite/AsyncSQLiteConnect.h"
#endif

namespace hku {

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#if HKU_ENABLE_SQLITE
static bool isSQLite(DBConnectBase *db) {
    const SQLiteConnect *sqlite = dynamic_cast<SQLiteConnect *>(db);
    return sqlite != nullptr;
}

static bool isAsyncSQLite(AsyncDBConnectBase *db) {
    const AsyncSQLiteConnect *sqlite = dynamic_cast<AsyncSQLiteConnect *>(db);
    return sqlite != nullptr;
}
#endif

#if HKU_ENABLE_MYSQL
static bool isMySQL(DBConnectBase *db) {
    const MySQLConnect *mysql = dynamic_cast<MySQLConnect *>(db);
    return mysql != nullptr;
}

static bool isAsyncMySQL(AsyncDBConnectBase *db) {
    const AsyncMySQLConnect *mysql = dynamic_cast<AsyncMySQLConnect *>(db);
    return mysql != nullptr;
}
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
/*
 * Upgrade and create the database
 */
void HKU_UTILS_API DBUpgrade(const DBConnectPtr &driver, const char *module_name,
                             const std::vector<std::string> &upgrade_scripts, int start_version,
                             const char *create_script) {
    HKU_TRACE("check {} database version ...", module_name);

    // Create the module version table if it does not exist
    if (!driver->tableExist("module_version")) {
        bool need_create = true;
#if HKU_ENABLE_SQLITE
        if (need_create && isSQLite(driver.get())) {
            driver->exec(
              "CREATE TABLE `module_version` (`id` INTEGER PRIMARY KEY AUTOINCREMENT,`module` "
              "TEXT, "
              "`version` INTEGER NOT NULL);");
            need_create = false;
        }
#endif

#if HKU_ENABLE_MYSQL
        if (need_create && isMySQL(driver.get())) {
            driver->exec(
              R"(CREATE TABLE `module_version` (
  `id` int NOT NULL AUTO_INCREMENT,
  `module` varchar(20) DEFAULT NULL,
  `version` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;)");
            need_create = false;
        }
#endif
    }

    // Return directly when there is neither an upgrade script nor a creation script
    if (upgrade_scripts.empty() && !create_script) {
        // Do nothing
        return;
    }

    // Try to get the template database version
    int version = 0;
    try {
        version = driver->queryInt(
          fmt::format("select `version` from `module_version` where module=\"{}\" limit 1",
                      module_name),
          0);
    } catch (...) {
        // Do noting
    }

    // When the module database version is 0 (the module database does not exist), create the
    // database if a creation script is given, otherwise return directly
    if (0 == version) {
        if (!create_script) {
            return;
        }

        // Create the database and set the module database version to 1
        driver->exec(create_script);
        driver->exec(fmt::format(
          "INSERT INTO `module_version` (module, version) VALUES (\"{}\", 1);", module_name));
        version = 1;
    }

    // When the current version is smaller than the start version of the upgrade script by one, an
    // intermediate version upgrade script is missing and the upgrade cannot proceed
    if (version < start_version - 1) {
        HKU_ERROR("THe {} database is too old, can't upgrade!", module_name);
        return;
    }

    int upgrade_scripts_count = static_cast<int>(upgrade_scripts.size());

    // There is no upgrade script, return directly
    if (0 == upgrade_scripts_count) {
        return;
    }

    // The final version to be upgraded to
    int to_version = start_version + upgrade_scripts_count - 1;
    HKU_TRACE("current {} database version: {}", module_name, version);

    // The current version is already not lower than the target version, no upgrade is needed,
    // return directly
    if (version >= to_version) {
        HKU_TRACE("current version({}) greater the upgrade version({}), ignored!", version,
                  to_version);
        return;
    }

    // If the current version is smaller than the start version of the script, the script index
    // starts from 0; otherwise the execution starts from the upgrade script whose index is (the
    // current version - the start version of script 0) + 1
    int start_index = version < start_version ? 0 : version - start_version + 1;
    HKU_TRACE("update {} database ..., update script index: {}", module_name, start_index);
    for (int i = start_index; i < upgrade_scripts_count; i++) {
        driver->exec(upgrade_scripts[i]);
    }

    driver->exec(fmt::format("UPDATE module_version SET `version`={} where `module`=\"{}\"",
                             to_version, module_name));
}

/*
 * The asynchronous version: upgrade and create the database
 */
net::awaitable<void> HKU_UTILS_API DBUpgrade(const AsyncDBConnectPtr &driver,
                                             const char *module_name,
                                             const std::vector<std::string> &upgrade_scripts,
                                             int start_version, const char *create_script) {
    HKU_TRACE("check {} database version ...", module_name);

    // Create the module version table if it does not exist
    if (!(co_await driver->tableExist("module_version"))) {
        bool need_create = true;
#if HKU_ENABLE_SQLITE
        if (need_create && isAsyncSQLite(driver.get())) {
            co_await driver->exec(
              "CREATE TABLE `module_version` (`id` INTEGER PRIMARY KEY AUTOINCREMENT,`module` "
              "TEXT, "
              "`version` INTEGER NOT NULL);");
            need_create = false;
        }
#endif

#if HKU_ENABLE_MYSQL
        if (need_create && isAsyncMySQL(driver.get())) {
            co_await driver->exec(
              R"(CREATE TABLE `module_version` (
  `id` int NOT NULL AUTO_INCREMENT,
  `module` varchar(20) DEFAULT NULL,
  `version` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;)");
            need_create = false;
        }
#endif
    }

    // Return directly when there is neither an upgrade script nor a creation script
    if (upgrade_scripts.empty() && !create_script) {
        co_return;
    }

    // Try to get the template database version
    int version = 0;
    try {
        version = co_await driver->queryInt(
          fmt::format("select `version` from `module_version` where module=\"{}\" limit 1",
                      module_name),
          0);
    } catch (...) {
        // Do noting
    }

    // When the module database version is 0 (the module database does not exist), create the
    // database if a creation script is given, otherwise return directly
    if (0 == version) {
        if (!create_script) {
            co_return;
        }

        // Create the database and set the module database version to 1
        co_await driver->exec(create_script);
        co_await driver->exec(fmt::format(
          "INSERT INTO `module_version` (module, version) VALUES (\"{}\", 1);", module_name));
        version = 1;
    }

    // When the current version is smaller than the start version of the upgrade script by one, an
    // intermediate version upgrade script is missing and the upgrade cannot proceed
    if (version < start_version - 1) {
        HKU_ERROR("THe {} database is too old, can't upgrade!", module_name);
        co_return;
    }

    int upgrade_scripts_count = static_cast<int>(upgrade_scripts.size());

    // There is no upgrade script, return directly
    if (0 == upgrade_scripts_count) {
        co_return;
    }

    // The final version to be upgraded to
    int to_version = start_version + upgrade_scripts_count - 1;
    HKU_TRACE("current {} database version: {}", module_name, version);

    // The current version is already not lower than the target version, no upgrade is needed,
    // return directly
    if (version >= to_version) {
        HKU_TRACE("current version({}) greater the upgrade version({}), ignored!", version,
                  to_version);
        co_return;
    }

    // If the current version is smaller than the start version of the script, the script index
    // starts from 0; otherwise the execution starts from the upgrade script whose index is (the
    // current version - the start version of script 0) + 1
    int start_index = version < start_version ? 0 : version - start_version + 1;
    HKU_TRACE("update {} database ..., update script index: {}", module_name, start_index);
    for (int i = start_index; i < upgrade_scripts_count; i++) {
        co_await driver->exec(upgrade_scripts[i]);
    }

    co_await driver->exec(fmt::format(
      "UPDATE module_version SET `version`={} where `module`=\"{}\"", to_version, module_name));
}

}  // namespace hku