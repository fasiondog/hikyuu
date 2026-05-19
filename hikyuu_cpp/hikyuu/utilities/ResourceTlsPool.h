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

/**
 * 线程局部资源池 - 完全无锁设计（Ring Buffer 实现）
 *
 * @details 使用 thread_local 存储，每个线程拥有独立的资源池实例。
 *          内部使用固定数组 + Ring Buffer 管理空闲资源，避免动态内存分配和锁竞争。
 *          由于资源完全隔离在线程内部，不需要任何锁或原子操作，性能最优。
 *
 *          **核心特性**：
 *          - 完全无锁：基于 thread_local，无需同步机制
 *          - Ring Buffer：使用固定数组实现循环队列，O(1) 时间复杂度的资源获取和归还
 *          - 零动态分配：编译期确定容量，运行时无需 malloc/free
 *          - 线程隔离：每个线程独立实例，避免跨线程竞争
 *
 *          **重要限制**：
 *          - 不适用于协程环境：协程可能在不同的线程间迁移执行，导致资源跨线程归还
 *          - 跨线程归还时资源会被直接删除并输出警告日志，无法实现资源复用
 *          - 如果需要在协程环境中使用资源池，建议使用全局共享池（如 ResourceAsioPool）
 *          - 仅适用于传统多线程模型，确保资源在同一线程内获取和释放
 *
 *          **跨线程安全检查机制**：
 *          - 资源获取时会记录所有者线程 ID
 *          - 资源归还时检查当前线程是否与所有者线程一致
 *          - 如果不一致：直接删除资源并输出警告日志，避免访问 thread_local 变量引发未定义行为
 *          - 如果一致：正常归还到池中实现复用
 *
 * @tparam ResourceType 资源类型，必须支持构造函数 ResourceType(const Parameter&)
 * @tparam MAX_POOL_SIZE_LIMIT 物理容量上限（编译期固定），默认值为 2
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
 * // 步骤3a：同步获取资源（推荐用法）
 * auto resource = pool.get();
 * if (resource) {
 *     resource->doWork();
 * } // 离开作用域时自动归还到池
 *
 * // 步骤3b：创建独立资源（不归入池管理）
 * auto standalone = pool.createStandalone();
 * if (standalone) {
 *     standalone.value()->doWork();
 * } // 离开作用域时自动 delete
 * @endcode
 *
 * @note 构造函数为私有，必须通过 init() + getInstance() 模式使用
 * @note 每个线程有独立的资源池实例，线程间不共享资源
 * @note 内部使用 Ring Buffer 管理空闲资源指针，避免 std::vector 的动态扩容
 * @note 不支持 asyncGet() 等异步方法，因为 TLS 资源获取本身就是同步且快速的
 * @warning 不建议在协程环境中使用，协程的线程迁移会导致资源无法复用
 */
namespace hku {

template <typename ResourceType, size_t MAX_POOL_SIZE_LIMIT = 2>
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
        std::thread::id owner_thread_id;  // 记录获取资源时的线程ID

        void operator()(ResourceType *resource) const {
            if (!resource) {
                return;
            }

            auto current_thread = std::this_thread::get_id();
            if (current_thread != owner_thread_id) {
                // 跨线程归还：直接删除资源，避免访问 thread_local 变量
                HKU_WARN(
                  "Resource returned from different thread, deleting directly to avoid "
                  "undefined behavior");
                delete resource;
            } else if (pool) {
                // 同线程归还：正常归还到池
                pool->returnResource(resource);
            } else {
                // pool 为空（可能在析构期间），直接删除
                delete resource;
            }
        }
    };

    /** 资源实例指针类型（使用 shared_ptr 管理生命周期） */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * 获取可用资源（同步操作）
     *
     * @return ResourcePtr 的 expected 对象，成功时包含资源指针，失败时包含错误信息
     *
     * @note 完全无锁操作，性能极高（O(1) 时间复杂度）
     * @note 返回的 shared_ptr 在析构时会自动归还资源到池（通过自定义删除器）
     * @note 如果当前资源数已达上限且无空闲资源，返回错误信息
     * @note 这是同步方法，在同一线程内调用立即返回，无需等待
     * @note 优先从 Ring Buffer 的空闲列表获取资源，若无则创建新资源
     *
     * @par 执行流程
     * 1. 检查 Ring Buffer 是否有空闲资源（m_freeCount > 0）
     *    - 有：从头部出队，返回资源指针
     * 2. 若无空闲资源，检查是否达到最大资源数限制（m_count >= m_maxCount）
     *    - 是：返回错误
     * 3. 创建新资源并增加计数
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. 尝试从空闲列表获取（Ring Buffer 头部出队）
        if (m_freeCount > 0) {
            ResourceType *p = m_resourceList[m_head];
            m_resourceList[m_head] = nullptr;
            m_head = (m_head + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount--;
            return ResourcePtr(p, ResourceDeleter{this, std::this_thread::get_id()});
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
        return ResourcePtr(p, ResourceDeleter{this, std::this_thread::get_id()});
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

    /**
     * 释放当前所有的空闲资源
     *
     * @note 遍历 Ring Buffer 中的所有空闲资源并删除，将 m_count 减去 m_freeCount
     * @note 此操作会立即释放所有未被使用的资源，但不会影响正在使用的资源
     * @note 适用于需要主动回收内存的场景（如程序空闲期）
     * @note 时间复杂度：O(m_freeCount)，需要遍历所有空闲资源
     */
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
    /**
     * 归还资源到池（Ring Buffer 尾部入队）
     *
     * @param p 待归还的资源指针
     *
     * @note 此方法由 ResourceDeleter 自动调用，用户无需手动调用
     * @note Ring Buffer 实现：将资源指针放入尾部位置，然后移动 tail 索引
     * @note 如果 Ring Buffer 已满（m_freeCount == MAX_POOL_SIZE_LIMIT），直接删除资源
     * @note 时间复杂度：O(1)，无锁操作
     *
     * @par Ring Buffer 工作原理
     * - 使用固定大小的数组 m_resourceList[MAX_POOL_SIZE_LIMIT]
     * - m_head 指向下一个可出队的位置（获取资源时从这里取）
     * - m_tail 指向下一个可入队的位置（归还资源时放到这里）
     * - 通过模运算实现循环：index = (index + 1) % MAX_POOL_SIZE_LIMIT
     * - 当 head == tail 且 m_freeCount == 0 时，表示空队列
     * - 当 m_freeCount == MAX_POOL_SIZE_LIMIT 时，表示满队列
     */
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