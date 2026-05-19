/*
 * ResourceHybridPool.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2025-05-12
 *      Author: fasiondog
 */
#pragma once
#ifndef HKU_UTILS_RESOURCE_HYBRID_POOL_H
#define HKU_UTILS_RESOURCE_HYBRID_POOL_H

#include <mutex>
#include "ResourceTlsPool.h"
#include "ResourcePool.h"

namespace hku {

/**
 * 混合资源池 - 结合同步 TLS Pool 和同步 ResourcePool
 *
 * @details 提供两级资源获取策略：
 *          1. 优先从线程局部同步池（TLS Pool）快速获取资源（无锁、高性能）
 *          2. 如果 TLS Pool 不可用，则从全局共享池（ResourcePool）获取资源（带锁、支持多线程）
 *
 *          **重要限制**：
 *          - 不适用于协程环境：TLS Pool 在协程线程迁移时会导致资源跨线程归还失败
 *          - 仅提供同步接口，不支持 asyncGet() 等异步方法
 *          - 仅适用于传统多线程模型，确保资源在同一线程内获取和释放
 *          - 对于协程环境，建议使用纯全局共享池（如 ResourceAsioPool）
 *
 *          这种设计兼顾了性能和灵活性：
 *          - 普通同步代码使用 TLS Pool，获得最佳性能（无锁）
 *          - 当 TLS Pool 资源耗尽时，自动降级到全局共享池（有锁但支持多线程共享）
 *          - 全局池使用 std::mutex 保护，确保线程安全
 *
 * @tparam ResourceType 资源类型，必须支持构造函数 ResourceType(const Parameter&)
 * @tparam MAX_TLS_POOL_SIZE_LIMIT TLS 池物理容量上限，默认 2（编译期固定，用于 std::array 分配）
 * @ingroup Utilities
 *
 * @par 使用示例
 * @code
 * // 步骤1：初始化参数
 * Parameter param;
 * param.set("host", "localhost");
 * param.set("port", 3306);
 *
 * // 步骤2a：使用默认的 TLS 池大小（模板参数）
 * ResourceHybridPool<MyResource> pool(param, 64);  // 全局共享池最大 64 个资源
 *
 * // 步骤2b：运行时指定 TLS 池实际使用大小（不能超过模板参数）
 * ResourceHybridPool<MyResource, 64> pool(param, 64, 16);  // TLS 池实际使用 16，全局池 64
 *
 * // 步骤3：同步获取（优先 TLS Pool）
 * auto resource = pool.get();
 * if (resource) {
 *     resource->doWork();
 * }
 * @endcode
 *
 * @note 内部维护两个独立的资源池实例
 * @note TLS Pool 是线程局部的，每个线程有独立实例
 * @note TLS Pool 的实际使用大小可在构造时指定（通过 max_tls_pool_size 参数），但不能超过模板参数
 * MAX_TLS_POOL_SIZE_LIMIT
 * @note Global Pool 的大小也可在构造时指定，不受模板参数限制
 * @warning 不建议在协程环境中使用，协程的线程迁移会导致 TLS Pool 资源无法复用
 */
template <typename ResourceType, size_t MAX_TLS_POOL_SIZE_LIMIT = 2>
class ResourceHybridPool {
public:
    /** TLS Pool 类型别名 */
    using TlsPoolType = ResourceTlsPool<ResourceType, MAX_TLS_POOL_SIZE_LIMIT>;

    /** 全局共享池类型别名（使用 std::mutex 支持多线程并发访问） */
    using GlobalPoolType = ResourcePool<ResourceType>;

    /**
     * 构造函数
     *
     * @param param 资源创建参数
     * @param max_tls_pool_size TLS 池实际使用的最大资源数，默认等于模板参数 MAX_TLS_POOL_SIZE_LIMIT
     *                          此值不能超过 MAX_TLS_POOL_SIZE_LIMIT，否则会被截断
     *                          如果设置为 0，则完全禁用 TLS Pool，所有请求都从全局共享池获取
     * @param max_global_pool_size 全局共享池（Global Pool）的最大资源数，默认 64
     *
     * @note TLS 池的实际大小由 max_tls_pool_size 控制，但底层数组容量仍为 MAX_TLS_POOL_SIZE_LIMIT
     * @note 如果 max_tls_pool_size > MAX_TLS_POOL_SIZE_LIMIT，会自动调整为 MAX_TLS_POOL_SIZE_LIMIT
     * @note 如果 max_tls_pool_size == 0，TLS Pool 被禁用，直接降级到全局共享池
     */
    explicit ResourceHybridPool(const Parameter &param,
                                size_t max_tls_pool_size = MAX_TLS_POOL_SIZE_LIMIT,
                                size_t max_global_pool_size = 64)
    : m_max_global_pool_size(max_global_pool_size), m_max_tls_pool_size(max_tls_pool_size) {
        // 检查并截断 TLS 池大小
        if (m_max_tls_pool_size > MAX_TLS_POOL_SIZE_LIMIT) {
            HKU_WARN("max_tls_pool_size({}) exceeds physical limit ({}), truncated to {}",
                     m_max_tls_pool_size, MAX_TLS_POOL_SIZE_LIMIT, MAX_TLS_POOL_SIZE_LIMIT);
            m_max_tls_pool_size = MAX_TLS_POOL_SIZE_LIMIT;
        }

        // 初始化 TLS Pool 的默认参数
        TlsPoolType::init(param);

        // 设置 TLS Pool 的实际使用大小（通过 getInstance() 后调用 maxCount）
        TlsPoolType::getInstance().maxCount(m_max_tls_pool_size);

        // 创建全局共享池（使用运行时指定的大小，支持多线程并发）
        m_global_pool = std::make_unique<GlobalPoolType>(param, m_max_global_pool_size);
    }

    /**
     * 析构函数
     */
    ~ResourceHybridPool() = default;

    /** 禁止拷贝 */
    ResourceHybridPool(const ResourceHybridPool &) = delete;
    ResourceHybridPool &operator=(const ResourceHybridPool &) = delete;

    /** 允许移动 */
    ResourceHybridPool(ResourceHybridPool &&) noexcept = default;
    ResourceHybridPool &operator=(ResourceHybridPool &&) noexcept = default;

    /**
     * 获取可用资源，如超出允许的最大资源数将返回空指针
     *
     * @return ResourcePtr 资源指针，失败时返回 nullptr
     *
     * @note 获取策略：
     *       1. 如果 TLS Pool 被禁用，直接从全局池获取
     *       2. 否则优先从 TLS Pool 获取（快速路径，无锁）
     *       3. 如果 TLS Pool 失败，自动降级到全局共享池
     *       4. 如果全局池也失败，返回 nullptr
     *
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    std::shared_ptr<ResourceType> get() {
        // 如果 TLS Pool 被禁用，直接从全局池获取
        if (m_max_tls_pool_size == 0) {
            return m_global_pool->get();
        }

        // 优先从 TLS Pool 获取（快速路径）
        auto tls_result = TlsPoolType::getInstance().get();
        if (tls_result) {
            return tls_result.value();  // TLS Pool 成功，提取 shared_ptr
        }

        // TLS Pool 失败，尝试从全局共享池获取
        return m_global_pool->get();  // 可能返回 nullptr 或抛出 CreateResourceException
    }

    /**
     * 在指定的超时时间内获取可用资源
     *
     * @param ms_timeout 超时时间，单位毫秒
     * @return ResourcePtr 资源指针
     *
     * @note 获取策略：
     *       1. 首先尝试从 TLS Pool 获取（快速路径，无锁）
     *       2. 如果 TLS Pool 失败，自动降级到全局共享池并等待指定时间
     *       3. 如果超时或创建失败，抛出 GetResourceTimeoutException
     *
     * @exception GetResourceTimeoutException 超时或资源耗尽
     * @exception CreateResourceException 新资源创建失败
     */
    std::shared_ptr<ResourceType> getWaitFor(uint64_t ms_timeout) {
        // 如果 TLS Pool 被禁用（max_tls_pool_size == 0），直接从全局池等待获取
        if (m_max_tls_pool_size == 0) {
            return m_global_pool->getWaitFor(ms_timeout);  // 可能抛出异常
        }

        // 优先从 TLS Pool 获取（快速路径）
        auto tls_result = TlsPoolType::getInstance().get();
        if (tls_result) {
            return tls_result.value();  // TLS Pool 成功，提取 shared_ptr
        }

        // TLS Pool 失败，尝试从全局共享池等待获取（带超时）
        return m_global_pool->getWaitFor(ms_timeout);  // 可能抛出 GetResourceTimeoutException
    }

    /**
     * 获取可用资源，如超出允许的最大资源数，将阻塞等待直到获得空闲资源
     *
     * @return ResourcePtr 资源指针
     *
     * @note 获取策略：
     *       1. 首先尝试从 TLS Pool 获取（快速路径，无锁）
     *       2. 如果 TLS Pool 失败，自动降级到全局共享池并无限期等待空闲资源
     *       3. 如果创建资源失败，抛出 CreateResourceException
     *
     * @note 此方法会阻塞当前线程直到获取到资源
     * @note 谨慎使用：如果资源池已满且没有资源归还，将永久阻塞
     * @note 适用于必须获取资源的场景，但要确保资源最终会被归还
     *
     * @exception GetResourceTimeoutException 理论上不会超时（ms_timeout=0），但可能因其他原因抛出
     * @exception CreateResourceException 新资源创建失败
     */
    std::shared_ptr<ResourceType> getAndWait() {
        return getWaitFor(0);  // 调用 getWaitFor(0) 实现无限期等待
    }

    /**
     * 仅从 TLS Pool 获取资源（同步）
     *
     * @return ResourcePtr 资源指针，失败时返回 nullptr
     */
    std::shared_ptr<ResourceType> getFromTlsPool() {
        auto result = TlsPoolType::getInstance().get();
        return result ? result.value() : nullptr;
    }

    /**
     * 仅从全局共享池获取资源（同步）
     *
     * @return ResourcePtr 资源指针，失败时返回 nullptr
     *
     * @note 此方法可能抛出 CreateResourceException
     */
    std::shared_ptr<ResourceType> getFromGlobalPool() {
        return m_global_pool->get();  // 可能返回 nullptr 或抛出 CreateResourceException
    }

    /** 获取 TLS Pool 引用 */
    TlsPoolType &tlsPool() {
        return TlsPoolType::getInstance();
    }

    /** 获取全局共享池引用 */
    GlobalPoolType &globalPool() {
        return *m_global_pool;
    }

    /** 获取 TLS 池实际使用的最大资源数 */
    size_t maxTlsPoolSize() const {
        return m_max_tls_pool_size;
    }

    /** 获取全局共享池最大资源数 */
    size_t maxGlobalPoolSize() const {
        return m_max_global_pool_size;
    }

private:
    size_t m_max_global_pool_size{64};  // 全局共享池最大资源数
    size_t m_max_tls_pool_size{
      MAX_TLS_POOL_SIZE_LIMIT};                     // TLS 池实际使用的最大资源数（不超过模板参数）
    std::unique_ptr<GlobalPoolType> m_global_pool;  // 全局共享池实例
};

}  // namespace hku

#endif /* RESOURCE_HYBRID_POOL_H */
