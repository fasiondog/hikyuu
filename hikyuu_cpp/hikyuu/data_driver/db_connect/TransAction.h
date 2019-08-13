/*
 * TransAction.h
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
 *      TransAction trans(driver);
 *      ...
 *  } catch (...) {
 *      driver->rollback();
 *  }
 * @endcode
 * @note 事务提交失败时，这里并没有处理回滚
 * @ingroup DataDriver
 */
class TransAction {
public:
    TransAction(const DBConnectPtr& driver) : m_driver(driver) {
        m_driver->transaction();
    }

    ~TransAction() {
        m_driver->commit();
    }

private:
    TransAction() = delete;
    TransAction(const TransAction&) = delete;
    TransAction& operator=(const TransAction&) = delete;    

private:
    DBConnectPtr m_driver;
};

} /* namespace */

#endif /* HIKYUU_DB_CONNECT_TRANSACTION_H */
