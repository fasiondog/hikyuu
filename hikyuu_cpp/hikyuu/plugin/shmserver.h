/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include "hikyuu/DataType.h"  // 提供 HKU_API 空回退定义（mac/linux 不注入 -D HKU_API）

namespace hku {

/**
 * 在当前进程内启动 shm 数据服务（加载 shmserver 插件并拉起服务端）
 * @details 范式同 startDataServer：门面先置 server 角色（防自连接，见设计 §5.5），再经
 * StockManager 加载 shmserver 插件并调用其 start()。插件为 VIP 授权，未授权 / 未安装时返回 false。
 * @param datadir 数据目录，为空时取 StockManager 当前数据目录
 * @param publish_shm 是否发布两类共享内存快照（K 线热数据 + 基础信息）
 * @param recv_spot 是否由本进程接收实时行情（内部调用 startSpotAgent，须在 init 之后）
 * @return 启动成功返回 true；本进程已处于客户端模式、插件缺失或授权无效时返回 false
 * @note 须在主程序初始化（import hikyuu 默认完成）之后调用；早于 init 将因数据未就绪而失败
 * @ingroup DataDriver
 */
bool HKU_API startShmServer(const std::string& datadir = "", bool publish_shm = true,
                            bool recv_spot = true) noexcept;

/**
 * 停止当前进程内的 shm 数据服务
 * @details 必须早于 nng_fini()：服务端 nng worker 持有在飞接收操作，若留待 nng 全局状态拆除后
 * 再由取消回调触发，会在已销毁的内部结构上重新装载接收而崩溃。由 GlobalInitializer::clean() 调用。
 * @ingroup DataDriver
 */
void HKU_API stopShmServer() noexcept;

/**
 * 查询当前进程内 shm 数据服务是否在运行
 * @return 服务在运行返回 true；插件未加载 / 未启动返回 false
 * @ingroup DataDriver
 */
bool HKU_API isShmServerRunning() noexcept;

}  // namespace hku
