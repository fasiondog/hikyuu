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
 * Automatic transaction handling: it starts a transaction automatically in the code block and
 * commits it automatically after the code block exits
 * @note When there are multiple data changes, an exception thrown in the middle of the program
 *       processing may cause the data to be partially committed
 * @ingroup DBConnect
 */
class AutoTransAction {
public:
    /**
     * Constructor
     * @param driver the database connection pointer
     */
    explicit AutoTransAction(const DBConnectPtr& driver) : m_driver(driver) {
        m_driver->transaction();
    }

    /** Destructor */
    ~AutoTransAction() {
        try {
            m_driver->commit();
        } catch (...) {
            HKU_ERROR("Transaction commit failed!");
            m_driver->rollback();
            m_driver.reset();
        }
    }

    /** Get the database connection */
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
 * Manual transaction handling; it allows the nested starting of the transaction and requires a
 * manual start and commit; on the automatic exit it does not commit automatically but rolls back!
 * @details There must be one effective manual transaction start; multiple nested starts are
 * regarded as one transaction handling. After a manual commit, the transaction must be started
 * manually again if there is new transaction handling.
 * @note When nested, it may happen that the inner transaction has been committed but the outer
 *       transaction handling fails and is rolled back (only the part of the corresponding
 * transaction handling is rolled back)
 * @ingroup DBConnect
 */
class TransAction {
public:
    /**
     * Constructor
     * @param driver the database connection pointer
     */
    explicit TransAction(const DBConnectPtr& driver)
    : m_driver(driver), m_committed(false), m_started(true) {
        HKU_CHECK(driver, "Null DBConnectPtr!");
        m_driver->transaction();
    }

    /** Destructor */
    ~TransAction() {
        // If the transaction has not been committed actively it is regarded as needing a rollback
        if (m_started && !m_committed) {
            HKU_WARN("The transaction is rolled back!");
            m_driver->rollback();
        } else if (!m_committed) {
            HKU_WARN("Not manul begin transaction!");
        }
    }

    /** Get the database connection */
    const DBConnectPtr& connect() const {
        return m_driver;
    }

    /** Start the transaction */
    void begin() {
        if (!m_started) {
            m_driver->transaction();
            m_committed = false;
            m_started = true;
        }
    }

    /** End and commit the transaction */
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
