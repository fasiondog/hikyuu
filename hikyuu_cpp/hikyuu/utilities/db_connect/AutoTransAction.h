/*
 * AutoTransAction.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_TRANSACTION_H
#define HIKYUU_DB_CONNECT_TRANSACTION_H

#include "DBConnectBase.h"

namespace hku {

/**
 * 自动事务处理，在代码块中自动启动事务，并在代码块退出后自动提交
 * @note 当有多个数据更改是，如果在程序处理中间发送异常时，可能导致数据被部分提交
 * @ingroup DBConnect
 */
class AutoTransAction {
public:
    /**
     * 构造函数
     * @param driver 数据库连接指针
     */
    explicit AutoTransAction(const DBConnectPtr& driver) : m_driver(driver) {
        m_driver->transaction();
    }

    /** 析构函数 */
    ~AutoTransAction() {
        try {
            m_driver->commit();
        } catch (...) {
            HKU_ERROR("Transaction commit failed!");
            m_driver->rollback();
            m_driver.reset();
        }
    }

    /** 获取数据库连接 */
    const DBConnectPtr& connect() const {
        return m_driver;
    }

private:
    AutoTransAction() = delete;
    AutoTransAction(const AutoTransAction&) = delete;
    AutoTransAction& operator=(const AutoTransAction&) = delete;

private:
    DBConnectPtr m_driver;
};

/**
 * 手动事务处理，允许嵌套启动事务，必须手工启动和提交事务，自动退出时不会自动提交事务，而是回滚！
 * @details 必须有一次有效的手工启动事务，多次嵌套启动事务将被视为一次事务处理。
 *          手工提交一次事务后，如有新的事务处理，须再次手工启动事务。
 * @note 嵌套时，可能发送内部事务已提交，但外部事务处理失败被回滚（仅回滚相应事务处理的部分）的情况
 * @ingroup DBConnect
 */
class TransAction {
public:
    /**
     * 构造函数
     * @param driver 数据库连接指针
     */
    explicit TransAction(const DBConnectPtr& driver)
    : m_driver(driver), m_committed(false), m_started(true) {
        HKU_CHECK(driver, "Null DBConnectPtr!");
        m_driver->transaction();
    }

    /** 析构函数 */
    ~TransAction() {
        // 如果没有主动提交事务，视为需要回滚
        if (m_started && !m_committed) {
            HKU_WARN("The transaction is rolled back!");
            m_driver->rollback();
        } else if (!m_committed) {
            HKU_WARN("Not manul begin transaction!");
        }
    }

    /** 获取数据库连接 */
    const DBConnectPtr& connect() const {
        return m_driver;
    }

    /** 启动事务 */
    void begin() {
        if (!m_started) {
            m_driver->transaction();
            m_committed = false;
            m_started = true;
        }
    }

    /** 结束并提交事务 */
    void end() {
        HKU_CHECK(m_started, "No transaction has started!");
        if (!m_committed) {
            m_driver->commit();
            m_committed = true;
            m_started = false;
        }
    }

private:
    TransAction() = delete;
    TransAction(const AutoTransAction&) = delete;
    TransAction& operator=(const AutoTransAction&) = delete;

private:
    DBConnectPtr m_driver;
    bool m_committed;
    bool m_started;
};

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_TRANSACTION_H */
