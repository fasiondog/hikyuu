/*
 * AsyncTransAction.h
 *
 *  Copyright (c) 2026, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_ASYNC_TRANSACTION_H
#define HIKYUU_DB_CONNECT_ASYNC_TRANSACTION_H

#include <memory>
#include "AsyncDBConnectBase.h"

namespace hku {

/**
 * 异步自动事务处理，在代码块中自动启动事务，并在代码块退出后自动提交
 * @note 当有多个数据更改时，如果在程序处理中间发送异常时，可能导致数据被部分提交
 * @details commit() 失败时会自动回滚，析构函数中如果未提交则自动回滚（使用 detached 协程）
 * @ingroup DBConnect
 */
class AsyncAutoTransAction {
public:
    /**
     * 工厂方法：创建实例并自动启动事务
     * @param driver 数据库连接指针
     * @return 异步事务对象
     */
    static net::awaitable<std::shared_ptr<AsyncAutoTransAction>> create(
      const AsyncDBConnectPtr& driver);

    /** 获取数据库连接 */
    const AsyncDBConnectPtr& connect() const {
        return m_driver;
    }

    /**
     * 提交事务
     * @note 如果提交失败，会自动尝试回滚，然后重新抛出原始异常
     */
    net::awaitable<void> commit();

    /** 析构函数：如果未提交则自动回滚 */
    ~AsyncAutoTransAction();

private:
    /** 私有构造函数 */
    explicit AsyncAutoTransAction(const AsyncDBConnectPtr& driver)
    : m_driver(driver), m_io_context(nullptr), m_committed(false) {
        HKU_CHECK(m_driver, "Null AsyncDBConnectPtr!");
    }

    /** 内部方法：启动事务 */
    net::awaitable<void> startTransaction() {
        // 获取当前协程环境的 io_context
        auto exec = co_await net::this_coro::executor;
        m_io_context = &static_cast<boost::asio::io_context&>(exec.context());

        // 启动事务
        co_await m_driver->transaction();
    }

private:
    AsyncDBConnectPtr m_driver;
    boost::asio::io_context* m_io_context = nullptr;
    bool m_committed = false;
};

// ============================================================================
// AsyncAutoTransAction 实现
// ============================================================================

inline net::awaitable<std::shared_ptr<AsyncAutoTransAction>> AsyncAutoTransAction::create(
  const AsyncDBConnectPtr& driver) {
    // 使用 shared_ptr 直接构造，绕过私有构造函数限制
    auto action = std::shared_ptr<AsyncAutoTransAction>(new AsyncAutoTransAction(driver));
    co_await action->startTransaction();
    co_return action;
}

inline net::awaitable<void> AsyncAutoTransAction::commit() {
    if (m_committed) {
        co_return;  // 已经提交过
    }

    bool rollback_needed = false;
    std::exception_ptr original_exception;

    try {
        co_await m_driver->commit();
        m_committed = true;
    } catch (...) {
        rollback_needed = true;
        original_exception = std::current_exception();
    }

    // 在 try-catch 外部处理回滚
    if (rollback_needed) {
        try {
            co_await m_driver->rollback();
        } catch (...) {
            HKU_ERROR("AsyncAutoTransAction: rollback failed after commit failure!");
        }

        // 重新抛出原始异常
        std::rethrow_exception(original_exception);
    }
}

inline AsyncAutoTransAction::~AsyncAutoTransAction() {
    if (!m_committed && m_driver && m_io_context) {
        // 启动一个 detached 协程来回滚（即发即忘）
        net::asio::co_spawn(
          *m_io_context,
          [driver = m_driver]() -> net::awaitable<void> {
              try {
                  co_await driver->rollback();
              } catch (...) {
                  HKU_WARN("AsyncAutoTransAction: rollback in destructor failed");
              }
          },
          net::asio::detached);
    }
}

/**
 * 异步手动事务处理，允许嵌套启动事务，必须手工启动和提交事务
 * @details 必须有一次有效的手工启动事务，多次嵌套启动事务将被视为一次事务处理。
 *          手工提交一次事务后，如有新的事务处理，须再次手工启动事务。
 * @note 析构时如果已启动但未提交，则自动回滚（使用 detached 协程）
 * @ingroup DBConnect
 */
class AsyncTransAction {
public:
    /**
     * 工厂方法：创建实例并自动启动事务
     * @param driver 数据库连接指针
     * @return 异步事务对象
     */
    static net::awaitable<std::shared_ptr<AsyncTransAction>> create(
      const AsyncDBConnectPtr& driver);

    /** 获取数据库连接 */
    const AsyncDBConnectPtr& connect() const {
        return m_driver;
    }

    /**
     * 启动事务（支持嵌套）
     * @note 如果已经启动，则不重复启动
     */
    net::awaitable<void> begin();

    /**
     * 结束并提交事务
     * @note 必须先调用 begin() 启动事务
     */
    net::awaitable<void> end();

    /** 回滚事务 */
    net::awaitable<void> rollback();

    /** 析构函数：如果已启动但未提交，则自动回滚 */
    ~AsyncTransAction();

private:
    /** 私有构造函数 */
    explicit AsyncTransAction(const AsyncDBConnectPtr& driver)
    : m_driver(driver), m_io_context(nullptr), m_committed(false), m_started(false) {
        HKU_CHECK(m_driver, "Null AsyncDBConnectPtr!");
    }

private:
    AsyncDBConnectPtr m_driver;
    net::asio::io_context* m_io_context = nullptr;
    bool m_committed = false;
    bool m_started = false;
};

// ============================================================================
// AsyncTransAction 实现
// ============================================================================

inline net::awaitable<std::shared_ptr<AsyncTransAction>> AsyncTransAction::create(
  const AsyncDBConnectPtr& driver) {
    // 使用 shared_ptr 直接构造，绕过私有构造函数限制
    auto action = std::shared_ptr<AsyncTransAction>(new AsyncTransAction(driver));
    co_await action->begin();
    co_return action;
}

inline net::awaitable<void> AsyncTransAction::begin() {
    if (!m_started) {
        // 获取当前协程环境的 io_context
        auto exec = co_await net::this_coro::executor;
        m_io_context = &static_cast<boost::asio::io_context&>(exec.context());

        // 启动事务
        co_await m_driver->transaction();
        m_started = true;
        m_committed = false;
    }
}

inline net::awaitable<void> AsyncTransAction::end() {
    HKU_CHECK(m_started, "No transaction has started!");

    if (!m_committed) {
        co_await m_driver->commit();
        m_committed = true;
        m_started = false;
    }
}

inline net::awaitable<void> AsyncTransAction::rollback() {
    if (m_started && !m_committed) {
        co_await m_driver->rollback();
        m_started = false;
        m_committed = false;
    }
}

inline AsyncTransAction::~AsyncTransAction() {
    // 如果没有主动提交事务，视为需要回滚
    if (m_started && !m_committed && m_driver && m_io_context) {
        HKU_WARN("AsyncTransAction: The transaction is rolled back in destructor!");

        // 启动一个 detached 协程来回滚（即发即忘）
        boost::asio::co_spawn(
          *m_io_context,
          [driver = m_driver]() -> net::awaitable<void> {
              try {
                  co_await driver->rollback();
              } catch (...) {
                  HKU_WARN("AsyncTransAction: rollback in destructor failed");
              }
          },
          boost::asio::detached);
    }
}

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_ASYNC_TRANSACTION_H */
