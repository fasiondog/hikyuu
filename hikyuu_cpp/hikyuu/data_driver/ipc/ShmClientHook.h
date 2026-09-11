/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#pragma once

#include <functional>
#include <string>
#include <vector>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"

namespace hku {
namespace ipc {

/**
 * 三条实时转发回调（shmserver 插件 connect 成功后注册、断开时注册空集注销）
 * @details 转发目标的实现（协议编解码、传输、共享内存读写）全部在 shmserver 插件内；核心库
 * 不持有任何插件接口类型，仅以本结构保存行情路径（Stock / dataserver）所需的转发函数，
 * 未注册（独立模式 / 未安装插件）时全部转发为安全空操作。
 */
struct ShmClientForwarders {
    /** Stock::realtimeUpdate 转发：服务端已应用返回 true */
    std::function<bool(const std::string& market_code, const KQuery::KType& ktype,
                       const KRecord& record)>
      realtimeUpdate;

    /** Stock::getLastUpdateTime 转发：返回服务端缓冲最后更新时刻，未注册 / 失败返回 Datetime::min() */
    std::function<Datetime(const std::string& market_code, const KQuery::KType& ktype)>
      getLastUpdateTime;

    /** 客户端委托服务进程从行情缓存服务（buffer server）拉取 K 线：服务端已受理返回 true */
    std::function<bool(const std::string& addr, const std::vector<std::string>& codes,
                       const KQuery::KType& ktype)>
      pullFromBufferServer;

    /** 是否为空（全部回调为空即视为注销状态） */
    inline explicit operator bool() const noexcept {
        return static_cast<bool>(realtimeUpdate) || static_cast<bool>(getLastUpdateTime) ||
               static_cast<bool>(pullFromBufferServer);
    }
};

/**
 * 注册 / 注销 shm 客户端转发回调（插件 connect 成功 / 断开时调用；传入空集合即注销）
 * @note 注册仅发生在 StockManager 初始化协商期，转发调用发生在行情线程；实现采用无锁发布-订阅
 * （发布-获取），转发侧只读注册期创建的稳定对象。回调对象生命周期随插件实例，插件销毁前须先注销
 */
HKU_API void registerShmClient(ShmClientForwarders fwd) noexcept;

/** 当前是否已注册客户端转发；未注册（独立模式 / 未安装插件）返回 false */
HKU_API bool shmClient() noexcept;

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
