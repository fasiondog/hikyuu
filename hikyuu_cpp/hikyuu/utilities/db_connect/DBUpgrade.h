/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#pragma once

#include <vector>
#include "DBConnectBase.h"

namespace hku {

/**
 * 升级及创建数据库
 * @param driver 数据库连接
 * @param module_name 模块名
 * @param upgrade_scripts 升级脚本数据数组（每个为一个版本的升级脚本）
 * @param start_version 升级脚本数组起始对应的版本
 * @param create_script 数据库创建脚本，若对应的数据库不存在则使用该脚本创建数据库
 * @ingroup DataDriver
 */
void HKU_UTILS_API DBUpgrade(const DBConnectPtr &driver, const char *module_name,
                             const std::vector<std::string> &upgrade_scripts, int start_version = 2,
                             const char *create_script = nullptr);

}  // namespace hku