/*
 * ResourceHybridVersionPool.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2025-05-13
 *      Author: fasiondog
 */
#pragma once
#ifndef HKU_UTILS_RESOURCE_HYBRID_VERSION_POOL_H
#define HKU_UTILS_RESOURCE_HYBRID_VERSION_POOL_H

#include <shared_mutex>
#include "ResourceTlsVersionPool.h"
#include "ResourceAsioPool.h"

namespace hku {

/**
 * @brief 混合版本资源池（TLS + 全局共享池 + 版本号管理）
 * @details 结合线程局部存储和全局共享池的优势，支持协程异步获取资源。
 *          当参数发生变化时，自动递增版本号并释放所有空闲的旧版本资源。
 *
 *          **重要约束**：ResourceType 必须实现 getVersion() 和 setVersion(int) 方法。
 *
 * @tparam ResourceType 资源类型，必须实现 getVersion() 和 setVersion(int) 方法
 * @tparam MAX_GLOBAL_POOL_SIZE_LIMIT TLS 池物理容量上限，默认 64（编译期固定，用于 std::array
 * 分配）
 * @ingroup Utilities
 *
 * @par 使用示例
 * @code
 * // 步骤1：初始化参数
 * Parameter param;
 * param.set("host", "localhost");
 * param.set("port", 3306);
 *
 * // 步骤2：创建混合版本池（可运行时指定全局共享池大小）
 * ResourceHybridVersionPool<MyResource> pool(param, 64);  // 全局共享池最大 64 个资源
 *
 * // 步骤3a：同步获取（优先 TLS Pool）
 * auto result = pool.get();
 * if (result) {
 *     result.value()->doWork();
 * }
 *
 * // 步骤3b：异步获取（协程中使用）
 * co_spawn(io_ctx, [&]() -> net::awaitable<void> {
 *     auto resource = co_await pool.asyncGet(std::chrono::seconds(5));
 *     if (resource) {
 *         resource->doWork();
 *     }
 * }, net::detached);
 *
 * // 步骤4：动态修改参数（触发版本递增）
 * pool.setParam<std::string>("host", "new_host");
 * // 下次获取时将自动使用新参数创建资源
 * @endcode
 *
 * @note 内部维护两个独立的资源池实例
 * @note TLS Pool 是线程局部的，每个线程有独立实例，大小可在构造时指定（不能超过模板参数）
 * @note Asio Pool 是全局共享的，支持跨线程访问，大小可在构造时指定
 * @note 版本号是全局的，所有线程共享
 */
template <typename ResourceType, size_t MAX_GLOBAL_POOL_SIZE_LIMIT = 64>
class ResourceHybridVersionPool {
public:
    /** TLS Pool 类型别名 */
    using TlsPoolType = ResourceTlsVersionPool<ResourceType, MAX_GLOBAL_POOL_SIZE_LIMIT>;

    /** 全局共享池类型别名（使用 std::shared_mutex 支持多线程并发读取） */
    using GlobalPoolType = ResourceAsioVersionPool<ResourceType, std::shared_mutex>;

    /**
     * 构造函数
     *
     * @param param 资源创建参数
     * @param max_tls_pool_size TLS 池实际使用的最大资源数，默认等于模板参数
     * MAX_GLOBAL_POOL_SIZE_LIMIT 此值不能超过 MAX_GLOBAL_POOL_SIZE_LIMIT，否则会被截断
     *                          如果设置为 0，则完全禁用 TLS Pool，所有请求都从全局共享池获取
     * @param max_global_pool_size 全局共享池（Asio Pool）的最大资源数，默认 64
     *
     * @note TLS 池的实际大小由 max_tls_pool_size 控制，但底层数组容量仍为
     * MAX_GLOBAL_POOL_SIZE_LIMIT
     * @note 如果 max_tls_pool_size > MAX_GLOBAL_POOL_SIZE_LIMIT，会自动调整为
     * MAX_GLOBAL_POOL_SIZE_LIMIT
     * @note 如果 max_tls_pool_size == 0，TLS Pool 被禁用，直接降级到全局共享池
     */
    explicit ResourceHybridVersionPool(const Parameter &param,
                                       size_t max_tls_pool_size = MAX_GLOBAL_POOL_SIZE_LIMIT,
                                       size_t max_global_pool_size = 64)
    : m_param(param),
      m_max_global_pool_size(max_global_pool_size),
      m_max_tls_pool_size(max_tls_pool_size),
      m_version(0) {
        // 检查并截断 TLS 池大小
        if (m_max_tls_pool_size > MAX_GLOBAL_POOL_SIZE_LIMIT) {
            HKU_WARN("max_tls_pool_size({}) exceeds physical limit ({}), truncated to {}",
                     m_max_tls_pool_size, MAX_GLOBAL_POOL_SIZE_LIMIT, MAX_GLOBAL_POOL_SIZE_LIMIT);
            m_max_tls_pool_size = MAX_GLOBAL_POOL_SIZE_LIMIT;
        }

        // 初始化 TLS Pool 的默认参数
        TlsPoolType::init(param);

        // 设置 TLS Pool 的实际使用大小
        TlsPoolType::getInstance().maxCount(m_max_tls_pool_size);

        // 创建全局共享池（使用运行时指定的大小，支持多线程并发）
        m_global_pool = std::make_unique<GlobalPoolType>(m_param, m_max_global_pool_size);
    }

    /**
     * 析构函数
     */
    ~ResourceHybridVersionPool() = default;

    /** 禁止拷贝 */
    ResourceHybridVersionPool(const ResourceHybridVersionPool &) = delete;
    ResourceHybridVersionPool &operator=(const ResourceHybridVersionPool &) = delete;

    /** 允许移动 */
    ResourceHybridVersionPool(ResourceHybridVersionPool &&) noexcept = default;
    ResourceHybridVersionPool &operator=(ResourceHybridVersionPool &&) noexcept = default;

    /**
     * 设置单个参数（触发版本递增）
     *
     * @tparam ValueType 参数值类型
     * @param name 参数名
     * @param value 参数值
     *
     * @note 此方法会：
     *       1. 更新全局参数
     *       2. 递增全局版本号
     *       3. 更新全局共享池的参数
     *       4. TLS Pool 的参数会在下次 get() 时懒更新
     *
     * @example
     * @code
     * pool.setParam<std::string>("host", "new_host");
     * pool.setParam<int>("port", 3307);
     * @endcode
     */
    template <typename ValueType>
    void setParam(const std::string &name, const ValueType &value) {
        // 更新全局参数和版本号
        {
            std::lock_guard<std::shared_mutex> lock(m_param_mutex);
            m_param.set<ValueType>(name, value);
        }
        m_version.fetch_add(1, std::memory_order_release);

        // 更新全局共享池的参数
        m_global_pool->template setParam<ValueType>(name, value);

        // 注意：TLS Pool 的参数会在下次 get() 时懒更新
    }

    /**
     * 整体替换参数（触发版本递增）
     *
     * @param param 新的参数对象
     *
     * @note 此方法会：
     *       1. 替换全局参数
     *       2. 递增全局版本号
     *       3. 更新全局共享池的参数
     *       4. TLS Pool 的参数会在下次 get() 时懒更新
     */
    void setParameter(const Parameter &param) {
        {
            std::lock_guard<std::shared_mutex> lock(m_param_mutex);
            m_param = param;
        }
        m_version.fetch_add(1, std::memory_order_release);
        m_global_pool->setParameter(param);
    }

    /**
     * 整体替换参数（移动语义，触发版本递增）
     *
     * @param param 新的参数对象（将被移动）
     */
    void setParameter(Parameter &&param) {
        {
            std::lock_guard<std::shared_mutex> lock(m_param_mutex);
            m_param = std::move(param);
        }
        m_version.fetch_add(1, std::memory_order_release);
        m_global_pool->setParameter(m_param);
    }

    /**
     * 检查参数是否存在
     *
     * @param name 参数名
     * @return true 如果参数存在
     */
    bool haveParam(const std::string &name) {
        std::shared_lock<std::shared_mutex> lock(m_param_mutex);
        return m_param.have(name);
    }

    /**
     * 获取参数值
     *
     * @tparam ValueType 参数值类型
     * @param name 参数名
     * @return 参数值
     * @throws std::exception 如果参数不存在或类型不匹配
     */
    template <typename ValueType>
    ValueType getParam(const std::string &name) {
        std::shared_lock<std::shared_mutex> lock(m_param_mutex);
        return m_param.get<ValueType>(name);
    }

    /**
     * 获取当前全局版本号
     *
     * @return 当前版本号
     */
    int getVersion() const {
        return m_version.load(std::memory_order_acquire);
    }

    /**
     * 手动递增版本号
     *
     * @note 通常在外部需要强制刷新资源时使用
     */
    void incVersion() {
        m_version.fetch_add(1, std::memory_order_release);
        m_global_pool->incVersion(1);
    }

    /**
     * 同步获取资源（两级策略：TLS Pool → Global Pool）
     *
     * @return ResourcePtr 的 expected 对象，成功时包含资源指针，失败时包含错误信息
     *
     * @note 获取策略：
     *       1. 检查 TLS Pool 版本是否需要同步
     *       2. 如果版本不同步，先更新 TLS Pool 的版本和参数
     *       3. 尝试从 TLS Pool 获取（快速路径，无锁）
     *       4. 如果 TLS Pool 失败，自动降级到全局共享池获取（同步方式）
     *       5. 如果两者都失败，返回组合错误信息
     *       6. 如需异步等待，请使用 asyncGet()
     *
     * @example
     * @code
     * auto result = pool.get();
     * if (result) {
     *     auto& resource = result.value();
     *     resource->doWork();
     * } else {
     *     HKU_ERROR("Failed to get resource: {}", result.error());
     * }
     * @endcode
     */
    stdx::expected<std::shared_ptr<ResourceType>, std::string> get() {
        // 如果 TLS Pool 被禁用（max_tls_pool_size == 0），直接从全局池获取
        if (m_max_tls_pool_size == 0) {
            return m_global_pool->get();
        }

        // 1. 检查 TLS Pool 版本是否需要同步
        auto &tls_pool = TlsPoolType::getInstance();
        int current_version = m_version.load(std::memory_order_acquire);

        if (tls_pool.getVersion() != current_version) {
            // 版本不匹配，同步参数和版本
            Parameter current_param;
            {
                std::shared_lock<std::shared_mutex> lock(m_param_mutex);
                current_param = m_param;
            }
            tls_pool.syncVersion(current_version, current_param);
        }

        // 2. 优先从 TLS Pool 获取（快速路径）
        auto tls_result = tls_pool.get();
        if (tls_result) {
            return tls_result;  // TLS Pool 成功，直接返回 shared_ptr
        }

        // 3. TLS Pool 失败，尝试从全局共享池获取（同步方式）
        auto global_result = m_global_pool->get();
        if (global_result) {
            return global_result;  // 全局池成功，返回 shared_ptr
        }

        // 4. 两者都失败，返回错误信息
        return stdx::unexpected(
          fmt::format("Both TLS and Global Pool exhausted. TLS: {}, Global: {}", tls_result.error(),
                      global_result.error()));
    }

    /**
     * 异步获取资源（优先从 TLS Pool，失败则从 Asio Pool）
     *
     * @param timeout 超时时间
     * @return expected<shared_ptr<ResourceType>, string>
     */
    net::awaitable<stdx::expected<std::shared_ptr<ResourceType>, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
        // 如果 TLS Pool 被禁用（max_tls_pool_size == 0），直接从全局池获取
        if (m_max_tls_pool_size == 0) {
            auto global_result = co_await m_global_pool->asyncGet(timeout);
            co_return global_result;
        }

        // 1. 检查 TLS Pool 版本是否需要同步
        auto &tls_pool = TlsPoolType::getInstance();
        int current_version = m_version.load(std::memory_order_acquire);

        if (tls_pool.getVersion() != current_version) {
            // 版本不匹配，同步参数和版本
            Parameter current_param;
            {
                std::shared_lock<std::shared_mutex> lock(m_param_mutex);
                current_param = m_param;
            }
            tls_pool.syncVersion(current_version, current_param);
        }

        // 2. 首先尝试从 TLS Pool 获取（快速路径）
        auto tls_result = tls_pool.get();
        if (tls_result) {
            // TLS Pool 成功，直接返回
            co_return tls_result;
        }

        // 3. TLS Pool 失败，从全局共享池获取（支持协程等待）
        auto global_result = co_await m_global_pool->asyncGet(timeout);
        co_return global_result;
    }

    /**
     * 仅从 TLS Pool 获取资源（同步）
     *
     * @return ResourcePtr 的 expected 对象
     */
    stdx::expected<std::shared_ptr<ResourceType>, std::string> getFromTlsPool() {
        // 先同步版本
        auto &tls_pool = TlsPoolType::getInstance();
        int current_version = m_version.load(std::memory_order_acquire);

        if (tls_pool.getVersion() != current_version) {
            Parameter current_param;
            {
                std::shared_lock<std::shared_mutex> lock(m_param_mutex);
                current_param = m_param;
            }
            tls_pool.syncVersion(current_version, current_param);
        }

        return tls_pool.get();
    }

    /**
     * 仅从全局共享池获取资源（异步）
     *
     * @param timeout 超时时间
     * @return awaitable<expected<shared_ptr<ResourceType>, string>>
     */
    net::awaitable<stdx::expected<std::shared_ptr<ResourceType>, std::string>> getFromGlobalPool(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
        auto result = co_await m_global_pool->asyncGet(timeout);
        co_return result;
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
    Parameter m_param;                                       // 全局参数（受锁保护）
    size_t m_max_global_pool_size{64};                       // 全局共享池最大资源数
    size_t m_max_tls_pool_size{MAX_GLOBAL_POOL_SIZE_LIMIT};  // TLS Pool 实际使用的最大资源数
    std::unique_ptr<GlobalPoolType> m_global_pool;           // 全局共享池实例
    std::atomic<int> m_version{0};                           // 全局版本号
    mutable std::shared_mutex m_param_mutex;                 // 保护参数访问的互斥锁
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_HYBRID_VERSION_POOL_H */