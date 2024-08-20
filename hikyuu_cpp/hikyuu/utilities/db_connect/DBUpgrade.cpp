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
#endif

#if HKU_ENABLE_SQLITE
#include "sqlite/SQLiteConnect.h"
#endif

namespace hku {

/*
 * 升级和创建数据库
 */
void HKU_UTILS_API DBUpgrade(const DBConnectPtr &driver, const char *module_name,
                             const std::vector<std::string> &upgrade_scripts, int start_version,
                             const char *create_script) {
    HKU_TRACE("check {} database version ...", module_name);

    // 如果模块版本表不存在，则创建该表
    if (!driver->tableExist("module_version")) {
        bool need_create = true;
#if HKU_ENABLE_SQLITE
        if (need_create && typeid(driver.get()) == typeid(SQLiteConnect *)) {
            driver->exec(
              "CREATE TABLE `module_version` (`id` INTEGER PRIMARY KEY AUTOINCREMENT,`module` "
              "TEXT, "
              "`version` INTEGER NOT NULL);");
            need_create = false;
        }
#endif

#if HKU_ENABLE_MYSQL
        if (need_create && typeid(driver.get()) == typeid(MySQLConnect *)) {
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

    // 如果没有升级脚本，也没有创建脚本，则直接返回
    if (upgrade_scripts.empty() && !create_script) {
        // Do nothing
        return;
    }

    // 尝试获取模板数据库版本
    int version = 0;
    try {
        version = driver->queryInt(
          fmt::format("select `version` from `module_version` where module=\"{}\" limit 1",
                      module_name),
          0);
    } catch (...) {
        // Do noting
    }

    // 模块数据库版本为0（不存在模块数据库）时，如果指定了数据库创建脚本，则创建数据库，否则直接返回
    if (0 == version) {
        if (!create_script) {
            return;
        }

        // 创建数据库，并将模块数据库版本设为1
        driver->exec(create_script);
        driver->exec(fmt::format(
          "INSERT INTO `module_version` (module, version) VALUES (\"{}\", 1);", module_name));
        version = 1;
    }

    // 当前版本号小于升级脚步对应的起始版本号还少1，说明缺失中间版本的升级脚本，无法升级
    if (version < start_version - 1) {
        HKU_ERROR("THe {} database is too old, can't upgrade!", module_name);
        return;
    }

    int upgrade_scripts_count = static_cast<int>(upgrade_scripts.size());

    // 不存在升级脚本，直接返回
    if (0 == upgrade_scripts_count) {
        return;
    }

    // 需要被升级到的最终版本
    int to_version = start_version + upgrade_scripts_count - 1;
    HKU_TRACE("current {} database version: {}", module_name, version);

    // 当前版本已经大于等于待升至的版本，无需升级，直接返回
    if (version >= to_version) {
        HKU_TRACE("current version({}) greater the upgrade version({}), ignored!", version,
                  to_version);
        return;
    }

    // 如果当前的版本号小于脚步对应的起始版本号，则从脚步索引从0开始；
    // 否则应从 (当前版本后 - 0号脚步对应的起始版本号) + 1 对应的升级脚步开始执行
    int start_index = version < start_version ? 0 : version - start_version + 1;
    HKU_TRACE("update {} database ..., update script index: {}", module_name, start_index);
    for (int i = start_index; i < upgrade_scripts_count; i++) {
        driver->exec(upgrade_scripts[i]);
    }

    driver->exec(fmt::format("UPDATE module_version SET `version`={} where `module`=\"{}\"",
                             to_version, module_name));
}

}  // namespace hku