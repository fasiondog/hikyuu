/*
 * hikyuu.h
 *
 *  Created on: 2011-11-13
 *      Author: fasiondog
 */

#ifndef HIKYUU_H_
#define HIKYUU_H_

#include "KData.h"
#include "StockManager.h"
#include "utilities/util.h"

namespace hku {

/**
 * Hikyuu核心初始化，客户端必须在程序入口处调用
 * @param config_file_name 配置信息文件名
 * @ingroup StockManage
 */
void HKU_API hikyuu_init(const string& config_file_name);


/**
 * 获取Hikyuu当前版本号
 * @return 版本号
 */
string HKU_API getVersion();


} /* namespace */

#endif /* HIKYUU_H_ */
