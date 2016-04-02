/*
 * hikyuu.h
 *
 *  Created on: 2011-11-13
 *      Author: fasiondog
 */

#ifndef HIKYUU_H_
#define HIKYUU_H_

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
 * 获取Stock，目的是封装StockManager，客户端不直接使用StockManager对象
 * @param querystr 格式：“市场简称证券代码”，如"sh000001"
 * @return 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
 */
Stock HKU_API getStock(const string& querystr);


} /* namespace */

#endif /* HIKYUU_H_ */
