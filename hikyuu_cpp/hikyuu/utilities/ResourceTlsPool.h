#pragma once
#ifndef HKU_UTILS_RESOURCE_TLS_POOL_H
#define HKU_UTILS_RESOURCE_TLS_POOL_H

#include <array>
#include <memory>
#include <optional>
#include <chrono>
#include <type_traits>
#include "expected.h"
#include "Parameter.h"
#include "Log.h"
#include "exception.h"
#include "net.h"

/**
 * 线程局部资源池 - 完全无锁设计（Ring Buffer 实现）
 *
 * @details 使用 thread_local 存储，每个线程拥有独立的资源池实例。
 *          内部使用固定数组 + Ring Buffer 管理空闲资源，避免动态内存分配。
 *          适用于协程环境，同一线程内的所有协程共享该线程的资源池。
 *          由于资源完全隔离在线程内部，不需要任何锁或原子操作，性能最优。
 *
 * @tparam ResourceType 资源类型，必须支持构造函数 ResourceType(const Parameter&)
 * @tparam MAX_POOL_SIZE_LIMIT 物理容量上限（编译期固定），默认值为 32
 * @ingroup Utilities
 *
 * @par 使用示例
 * @code
 * // 步骤1：初始化默认参数（程序启动时调用一次）
 * Parameter param;
 * param.set("host", "localhost");
 * param.set("port", 3306);
 * ResourceTlsPool<MyResource>::init(param);
 *
 * // 步骤2：获取线程局部资源池实例
 * auto& pool = ResourceTlsPool<MyResource>::getInstance();
 *
 * // 步骤3a：同步获取资源
 * auto resource = pool.get();
 * if (resource) {
 *     resource->doWork();
 * } // 离开作用域时自动归还到池
 *
 * // 步骤3b：协程中异步获取资源（带超时控制）
 * co_spawn(io_ctx, [&]() -> asio::awaitable<void> {
 *     auto& pool = ResourceTlsPool<MyResource>::getInstance();
 *     auto result = co_await pool.asyncGet(std::chrono::seconds(5));
 *     if (result) {
 *         result.value()->doWork();
 *     }
 * }, asio::detached);
 * @endcode
 *
 * @note 构造函数为私有，必须通过 init() + getInstance() 模式使用
 * @note 每个线程有独立的资源池实例，线程间不共享资源
 * @note 同一线程内的所有协程共享该线程的资源池
 * @note 内部使用 Ring Buffer 管理空闲资源指针，避免 std::vector 的动态扩容
 */
namespace hku {

// 使用 net 命名空间中的 asio 别名
namespace asio = net::asio;

template <typename ResourceType, size_t MAX_POOL_SIZE_LIMIT = 32>
class ResourceTlsPool {
public:
    /**
     * 初始化全局默认参数（可选）
     *
     * @param param 默认资源创建参数
     *
     * @note 应在程序启动时调用一次，设置全局默认值
     * @note 后续调用 getInstance() 无参版本时将使用这些默认值
     * @note 如果未调用此方法，将使用 Parameter{}
     * @note 最大资源数由模板参数 MAX_POOL_SIZE_LIMIT 决定（默认 32）
     *
     * @example
     * @code
     * // 程序启动时初始化
     * Parameter defaultParam;
     * defaultParam.set("host", "localhost");
     * defaultParam.set("port", 3306);
     * ResourceTlsPool<MyResource>::init(defaultParam);
     *
     * // 后续使用无需传参
     * auto& pool = ResourceTlsPool<MyResource>::getInstance();
     * @endcode
     */
    static void init(const Parameter &param) {
        ms_defaultParam = param;
    }

    /**
     * 获取当前线程的资源池实例（单例模式）
     *
     * @return 当前线程的资源池引用
     *
     * @note thread_local 保证每个线程有独立的资源池实例
     * @note 使用 init() 设置的默认参数创建实例，最大池大小由模板参数 MAX_POOL_SIZE_LIMIT 决定
     * @note 首次调用时创建实例，后续调用返回同一实例
     *
     * @example
     * @code
     * // 先初始化默认参数
     * ResourceTlsPool<MyResource>::init(param);
     *
     * // 后续直接获取，无需传参
     * auto& pool = ResourceTlsPool<MyResource>::getInstance();
     * @endcode
     */
    static ResourceTlsPool &getInstance() {
        thread_local static ResourceTlsPool instance(ms_defaultParam);
        return instance;
    }

    /**
     * 析构函数，释放当前线程的所有缓存资源
     */
    virtual ~ResourceTlsPool() {
        for (size_t i = 0; i < m_freeCount; ++i) {
            if (m_resourceList[i]) {
                delete m_resourceList[i];
                m_resourceList[i] = nullptr;
            }
        }
        m_freeCount = 0;
    }

    /** 资源删除器，用于 shared_ptr 自动归还资源 */
    struct ResourceDeleter {
        ResourceTlsPool *pool;

        void operator()(ResourceType *resource) const {
            if (resource && pool) {
                pool->returnResource(resource);
            } else if (resource) {
                delete resource;
            }
        }
    };

    /** 资源实例指针类型（使用 shared_ptr 管理生命周期） */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * 获取可用资源
     *
     * @return ResourcePtr 的 expected 对象，成功时包含资源指针，失败时包含错误信息
     *
     * @note 完全无锁操作，性能极高
     * @note 返回的 shared_ptr 在析构时会自动归还资源到池（通过自定义删除器）
     * @note 如果当前资源数已达上限且无空闲资源，返回错误信息
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. 尝试从空闲列表获取（Ring Buffer 头部出队）
        if (m_freeCount > 0) {
            ResourceType *p = m_resourceList[m_head];
            m_resourceList[m_head] = nullptr;
            m_head = (m_head + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount--;
            return ResourcePtr(p, ResourceDeleter{this});
        }

        // 2. 无空闲资源，检查是否可以创建新资源
        if (m_count >= m_maxCount) {
            return stdx::unexpected("No available resources and maximum pool size reached");
        }

        // 3. 创建新资源
        ResourceType *p = nullptr;
        try {
            p = new ResourceType(m_param);
        } catch (const std::exception &e) {
            return stdx::unexpected(fmt::format("Failed to create resource: {}", e.what()));
        } catch (...) {
            return stdx::unexpected("Failed to create resource: Unknown error");
        }

        m_count++;
        return ResourcePtr(p, ResourceDeleter{this});
    }

    /**
     * 创建独立管理的资源（不归入资源池）
     *
     * @return 资源指针的 expected 对象，成功时包含资源指针，失败时包含错误信息
     *
     * @note 返回的资源由调用者完全管理，不会归还到资源池
     * @note 资源析构时会直接 delete，而不是归还到池
     * @note 使用资源池内置的参数创建资源
     * @note 适用于需要临时资源或特殊配置的场景
     *
     * @example
     * @code
     * auto result = pool.createStandalone();
     * if (result) {
     *     auto res = std::move(result.value());
     *     res->doWork();
     * } // 离开作用域时自动 delete
     * @endcode
     */
    stdx::expected<std::unique_ptr<ResourceType>, std::string> createStandalone() {
        ResourceType *p = nullptr;
        try {
            p = new ResourceType(m_param);
        } catch (const std::exception &e) {
            return stdx::unexpected(
              fmt::format("Failed to create standalone resource: {}", e.what()));
        } catch (...) {
            return stdx::unexpected("Failed to create standalone resource: Unknown error");
        }

        // 返回标准的 unique_ptr，使用默认 deleter（直接 delete）
        return std::unique_ptr<ResourceType>(p);
    }

    /**
     * 异步获取可用资源（带超时控制）
     *
     * @param timeout 超时时间，默认 5 秒
     * @return ResourcePtr 的 expected 对象，成功时包含资源指针，失败时包含错误信息
     *
     * @note 在协程中使用：
     * @code
     * auto result = co_await pool.asyncGet(std::chrono::seconds(5));
     * if (result) {
     *     auto& res = result.value();
     *     res->doWork();
     * } else {
     *     HKU_ERROR("Failed to get resource: {}", result.error());
     * }
     * @endcode
     * @note 如果超时时仍无法获取资源，返回错误信息
     * @note 此方法适用于资源池已满且无空闲资源的场景，会轮询检查是否有资源被归还
     */
    asio::awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::milliseconds timeout = std::chrono::seconds(5)) {
        // 从当前协程上下文获取 executor
        auto executor = co_await asio::this_coro::executor;

        // 1. 尝试立即获取（快速路径）
        if (m_freeCount > 0) {
            ResourceType *p = m_resourceList[m_head];
            m_resourceList[m_head] = nullptr;
            m_head = (m_head + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount--;
            co_return ResourcePtr(p, ResourceDeleter{this});
        }

        // 2. 如果可以创建新资源，直接创建
        if (m_count < m_maxCount) {
            ResourceType *p = nullptr;
            try {
                p = new ResourceType(m_param);
            } catch (const std::exception &e) {
                co_return stdx::unexpected(fmt::format("Failed to create resource: {}", e.what()));
            } catch (...) {
                co_return stdx::unexpected("Failed to create resource: Unknown error");
            }

            m_count++;
            co_return ResourcePtr(p, ResourceDeleter{this});
        }

        // 3. 资源池已满，轮询等待资源归还
        // 使用短时间间隔（5ms）反复检查，避免长时间阻塞
        asio::steady_timer timer(executor);
        constexpr auto poll_interval = std::chrono::milliseconds(5);
        auto deadline = std::chrono::steady_clock::now() + timeout;

        while (std::chrono::steady_clock::now() < deadline) {
            // 短暂等待后再次检查
            timer.expires_after(poll_interval);
            co_await timer.async_wait(asio::use_awaitable);

            // 检查是否有空闲资源
            if (m_freeCount > 0) {
                ResourceType *p = m_resourceList[m_head];
                m_resourceList[m_head] = nullptr;
                m_head = (m_head + 1) % MAX_POOL_SIZE_LIMIT;
                m_freeCount--;
                co_return ResourcePtr(p, ResourceDeleter{this});
            }
        }

        // 超时仍未获取到资源，返回错误
        co_return stdx::unexpected("Timeout waiting for available resource");
    }

    /** 当前活动的资源数（含空闲及被使用的资源） */
    size_t count() const {
        return m_count;
    }

    /** 当前空闲的资源数 */
    size_t idleCount() const {
        return m_freeCount;
    }

    /** 获取允许的最大资源数（逻辑上限） */
    size_t maxCount() const {
        return m_maxCount;
    }

    /** 设置最大资源数（逻辑上限，不能超过物理容量） */
    void maxCount(size_t num) {
        if (num > MAX_POOL_SIZE_LIMIT) {
            HKU_WARN("maxCount({}) exceeds physical limit ({}), truncated to {}", num,
                     MAX_POOL_SIZE_LIMIT, MAX_POOL_SIZE_LIMIT);
            m_maxCount = MAX_POOL_SIZE_LIMIT;
        } else {
            m_maxCount = num;
        }
    }

    /** 释放当前所有的空闲资源 */
    void releaseIdleResource() {
        for (size_t i = 0; i < m_freeCount; ++i) {
            size_t idx = (m_head + i) % MAX_POOL_SIZE_LIMIT;
            if (m_resourceList[idx]) {
                delete m_resourceList[idx];
                m_resourceList[idx] = nullptr;
            }
        }
        m_count -= m_freeCount;
        m_freeCount = 0;
    }

private:
    ResourceTlsPool(const ResourceTlsPool &) = delete;
    ResourceTlsPool &operator=(const ResourceTlsPool &) = delete;

    /**
     * 构造函数（私有，仅通过 getInstance() 访问）
     *
     * @param param 资源创建参数
     */
    explicit ResourceTlsPool(const Parameter &param) : m_param(param) {
        m_resourceList.fill(nullptr);
    }

    /**
     * 默认构造函数（私有，仅通过 getInstance() 访问）
     */
    ResourceTlsPool() {
        m_resourceList.fill(nullptr);
    }

private:
    /** 归还资源到池（Ring Buffer 尾部入队） */
    void returnResource(ResourceType *p) {
        if (p && m_freeCount < MAX_POOL_SIZE_LIMIT) {
            m_resourceList[m_tail] = p;
            m_tail = (m_tail + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount++;
        } else if (p) {
            // 如果 Ring Buffer 已满，直接删除资源
            delete p;
        }
    }

private:
    size_t m_maxCount = MAX_POOL_SIZE_LIMIT;  // 逻辑资源上限（运行时可配置，不能超过物理容量）
    size_t m_count = 0;                       // 当前活动的资源数（含空闲和被使用）
    size_t m_freeCount = 0;                   // 空闲资源数量
    size_t m_head = 0;                        // Ring Buffer 头部索引（出队位置）
    size_t m_tail = 0;                        // Ring Buffer 尾部索引（入队位置）
    Parameter m_param;                        // 资源创建参数
    std::array<ResourceType *, MAX_POOL_SIZE_LIMIT>
      m_resourceList{};  // 空闲资源数组（Ring Buffer，物理容量固定）

private:
    static Parameter ms_defaultParam;
};

// Static member initialization
template <typename ResourceType, size_t MAX_POOL_SIZE_LIMIT>
Parameter ResourceTlsPool<ResourceType, MAX_POOL_SIZE_LIMIT>::ms_defaultParam{};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_TLS_POOL_H */