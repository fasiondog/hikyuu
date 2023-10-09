/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#include "DBUpgrade.h"
#include "../../debug.h"

namespace hku {

/*
 * 升级和创建数据库
 */
void HKU_API DBUpgrade(const DBConnectPtr &driver, const char *module_name,
                       const std::vector<string> &upgrade_scripts, int start_version,
                       const char *create_script) {
    HKU_TRACE("check {} database version ...", module_name);

    // 如果模块版本表不存在，则创建该表
    if (!driver->tableExist("module_version")) {
        driver->exec(
          "CREATE TABLE `module_version` (`id` INTEGER PRIMARY KEY AUTOINCREMENT,`module` TEXT NOT "
          "NULL, `version` INTEGER NOT NULL);");
    }

    // 尝试获取模板数据库版本
    int version = 0;
    try {
        version = driver->queryInt(format(
          "select `version` from `module_version` where module=\"{}\" limit 1", module_name));
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
        driver->exec(format("INSERT INTO `module_version` (module, version) VALUES (\"{}\", 1);",
                            module_name));
        version = 1;
    }

    // 缺失中间版本的升级脚本
    if (version < start_version - 1) {
        HKU_ERROR("THe {} database is too old, can't upgrade!", module_name);
        return;
    }

    size_t upgrade_scripts_count = upgrade_scripts.size();

    // 不存在升级脚本，直接返回
    if (0 == upgrade_scripts_count) {
        return;
    }

    // 需要被升级到的最终版本
    int to_version = start_version + upgrade_scripts_count - 1;
    HKU_TRACE("current {} database version: {}", module_name, version);

    if (version >= to_version) {
        return;
    }

    int start_index = start_version - version - 1;
    HKU_TRACE("update {} database ...", module_name);
    for (int i = start_index; i < upgrade_scripts_count; i++) {
        driver->exec(upgrade_scripts[i]);
    }

    driver->exec(format("UPDATE module_version SET `version`={} where `module`=\"{}\"", to_version,
                        module_name));
}

}  // namespace hku