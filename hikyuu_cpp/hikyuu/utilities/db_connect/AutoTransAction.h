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
 * @code
 *  try {
 *      AutoTransAction trans(driver);
 *      ...
 *  } catch (...) {
 *      driver->rollback();
 *  }
 * @endcode
 * @note 事务提交失败时，这里并没有处理回滚
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
 * 手动事务处理，允许嵌套提交事务，退出时自动提交
 * @details 必须有一次有效的手工启动事务，多次嵌套启动事务将被视为一次事务处理。
 *          手工提交一次事务后，如有新的事务处理，须再次手工启动事务。
 * @ingroup DBConnect
 */
class TransAction {
public:
    /**
     * 构造函数
     * @param driver 数据库连接指针
     */
    explicit TransAction(const DBConnectPtr& driver) : m_driver(driver), m_committed(true) {}

    /** 析构函数 */
    ~TransAction() {
        try {
            if (!m_committed) {
                m_driver->commit();
            }
        } catch (...) {
            HKU_ERROR("Transaction commit failed!");
            m_driver.reset();
        }
    }

    /** 获取数据库连接 */
    const DBConnectPtr& connect() const {
        return m_driver;
    }

    /** 启动事务 */
    void begin() {
        if (m_committed) {
            m_driver->transaction();
            m_committed = false;
        }
    }

    /** 结束并提交事务 */
    void end() {
        if (!m_committed) {
            m_driver->commit();
            m_committed = true;
        }
    }

    /** 回滚事务 */
    void rollback() {
        m_driver->rollback();
    }

private:
    TransAction() = delete;
    TransAction(const AutoTransAction&) = delete;
    TransAction& operator=(const AutoTransAction&) = delete;

private:
    DBConnectPtr m_driver;
    bool m_committed;
};

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_TRANSACTION_H */
