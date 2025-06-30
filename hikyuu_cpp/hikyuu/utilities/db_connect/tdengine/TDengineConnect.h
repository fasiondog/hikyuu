/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-04
 *      Author: fasiondog
 */

#pragma once
#ifndef HIYUU_DB_CONNECT_TDENGINE_CONNECT_H
#define HIYUU_DB_CONNECT_TDENGINE_CONNECT_H

#include "../DBConnectBase.h"
#include "TDengineStatement.h"

namespace hku {

class HKU_UTILS_API TDengineConnect : public DBConnectBase {
public:
    explicit TDengineConnect(const Parameter &param);
    virtual ~TDengineConnect();

    TDengineConnect(const TDengineConnect &) = delete;
    TDengineConnect &operator=(const TDengineConnect &) = delete;

    virtual bool ping() override;

    virtual int64_t exec(const std::string &sql_string) override;
    virtual SQLStatementPtr getStatement(const std::string &sql_statement) override;
    virtual bool tableExist(const std::string &tablename) override;

    virtual void resetAutoIncrement(const std::string &tablename) override {}

    virtual void transaction() override {}
    virtual void commit() override {}
    virtual void rollback() noexcept override {}

public:
    TAOS *getRawTAOS() const noexcept {
        return m_taos;
    }

    bool reconnect() noexcept;

private:
    void connect();
    void close();

private:
    TAOS *m_taos{nullptr};
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_TDENGINE_CONNECT_H */