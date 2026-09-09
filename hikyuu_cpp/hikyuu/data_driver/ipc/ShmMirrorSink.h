/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"

namespace hku {
namespace ipc {

/**
 * 实时镜像写入的插件侧实现接口（由 shmserver 插件实现并注册）
 * @details Stock::realtimeUpdate 经核心库薄转发（shmMirrorRealtimeUpdate）驱动本接口，
 * 使核心库无需知晓共享内存发布器的存在。发布窗口内的暂存 / 旧段双写 / 接管后重放均为
 * 本接口实现方的内部职责，核心库转发器不做任何暂存（见设计 §5.1）。
 * @note 本接口是核心库侧的回调契约（依赖倒置），非插件控制入口，故置于 data_driver/ipc/
 * 而非 plugin/interface/（后者清一色为继承 PluginBase 的插件控制接口）。
 * @ingroup DataDriver
 */
class HKU_API ShmMirrorSink {
public:
    virtual ~ShmMirrorSink() = default;
    virtual void onRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                  const KRecord& record) = 0;
};

/**
 * 注册 / 注销镜像实现（传 nullptr 注销）；由插件在 start / stop 单点调用
 * @note 注册时记录发布进程 pid，用于排除 fork 出的子进程写入（维持单写者前提）
 */
HKU_API void registerShmMirrorSink(ShmMirrorSink* sink) noexcept;

/**
 * Stock::realtimeUpdate 的既有调用入口；无注册者时仅一次原子读即返回
 * @details 核心库实现仅做原子快路径判断 + fork 子进程 pid 门控 + 虚调用转发
 */
HKU_API void shmMirrorRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                     const KRecord& record);

}  // namespace ipc
}  // namespace hku
