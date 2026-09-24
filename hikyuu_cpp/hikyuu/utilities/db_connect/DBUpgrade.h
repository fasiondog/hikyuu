/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#pragma once

#include <vector>
#include "DBConnectBase.h"
#include "AsyncDBConnectBase.h"

namespace hku {

/**
 * Upgrade and create the database
 * @param driver database connection
 * @param module_name module name
 * @param upgrade_scripts the upgrade script data array (every one is the upgrade script of one
 *                        version)
 * @param start_version the version corresponding to the start of the upgrade script array
 * @param create_script the database creation script; it is used to create the database when the
 *                      corresponding database does not exist
 * @ingroup DataDriver
 */
void HKU_UTILS_API DBUpgrade(const DBConnectPtr &driver, const char *module_name,
                             const std::vector<std::string> &upgrade_scripts, int start_version = 2,
                             const char *create_script = nullptr);

net::awaitable<void> HKU_UTILS_API DBUpgrade(const AsyncDBConnectPtr &driver,
                                             const char *module_name,
                                             const std::vector<std::string> &upgrade_scripts,
                                             int start_version = 2,
                                             const char *create_script = nullptr);

}  // namespace hku