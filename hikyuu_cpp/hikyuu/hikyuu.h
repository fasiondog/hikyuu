/*
 * hikyuu.h
 *
 *  Created on: 2011-11-13
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_H_
#define HIKYUU_H_

#include "KData.h"
#include "Stock.h"
#include "StockManager.h"
#include "utilities/util.h"
#include "utilities/Parameter.h"

namespace hku {

/**
 * @ingroup StockManage
 * @{
 */

/**
 * Hikyuu核心初始化，客户端必须在程序入口处调用
 * @param config_file_name 配置信息文件名
 * @param ignore_preload 忽略配置信息中的预加载设置，即不加载数据至内存。
 *                       用于某些场合启动hikyuu，但仅用于获取数据库的基本信息。
 */
void HKU_API hikyuu_init(const string& config_file_name, bool ignore_preload = false);

/**
 * 获取Hikyuu当前版本号
 * @return 版本号
 */
string HKU_API getVersion();

/**
 * 获取详细版本号，包含构建时间
 */
std::string HKU_API getVersionWithBuild();

/** @} */

}  // namespace hku

#endif /* HIKYUU_H_ */
