/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <functional>
#include <string>
#include <vector>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"
#include "hikyuu/plugin/interface/ShmClientInterface.h"

namespace hku {
namespace ipc {

/**
 * 注册 / 注销 shm 客户端实现（插件进入客户端模式时注册，传 nullptr 注销）
 * @details 核心库只保存裸指针（所有权归插件），未注册时全部转发为安全空操作，
 * 进程按独立模式运行。插件侧在 disconnect() 中自行清理内部状态，此处仅切断引用。
 * @note 原子量保证行情线程（转发调用方）与协商线程（注册方）之间的可见性
 */
HKU_API void registerShmClient(ShmClientInterface* client) noexcept;

/** 当前注册的客户端实现；未注册（独立模式 / 未安装插件）返回 nullptr */
HKU_API ShmClientInterface* shmClient() noexcept;

/**
 * 客户端实时更新转发（Stock::realtimeUpdate 调用）
 * @return true 服务端已应用 | false 未注册客户端、通讯失败或服务端未应用
 */
HKU_API bool forwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                   const KRecord& record);

/**
 * 客户端向服务进程查询指定证券×类型缓冲的最后更新时刻（Stock::getLastUpdateTime 调用）
 * @return 服务端缓冲的最后更新时刻；未注册客户端、通讯失败或证券不存在时返回 Datetime::min()
 */
HKU_API Datetime forwardGetLastUpdateTime(const std::string& market_code,
                                          const KQuery::KType& ktype);

/**
 * 客户端委托服务进程从行情缓存服务（buffer server）拉取最新 K 线并更新
 * @return true 服务端已受理 | false 未注册客户端或通讯失败（调用方仅记日志，不中断）
 */
HKU_API bool forwardPullFromBufferServer(const std::string& addr,
                                         const std::vector<std::string>& codes,
                                         const KQuery::KType& ktype);

/**
 * 设置长阻塞等待期间的中断检查器（如等待数据服务就绪），返回 true 表示应中断等待
 * @details Python 环境下由绑定层注册（重新获取 GIL 后检查挂起信号），使等待可被 Ctrl+C 打断。
 * 属核心库通用设施：等待逻辑整体在插件内，插件经 ipc::checkInterrupted() 回调此处注册的实现。
 */
HKU_API void setInterruptChecker(std::function<bool()> checker);

/** 调用已注册的中断检查器，未注册时返回 false */
HKU_API bool checkInterrupted();

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
