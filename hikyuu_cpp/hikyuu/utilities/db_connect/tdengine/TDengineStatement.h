/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-04
 *      Author: fasiondog
 */

#pragma once
#ifndef HIYUU_DB_CONNECT_TDENGINE_STATEMENT_H
#define HIYUU_DB_CONNECT_TDENGINE_STATEMENT_H

#include <string>
#include <vector>
#include <boost/any.hpp>
#include "../SQLStatementBase.h"
#include "TaosMacro.h"
#include "taos.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

class HKU_UTILS_API TDengineStatement : public SQLStatementBase {
public:
    TDengineStatement() = delete;
    TDengineStatement(DBConnectBase *driver, const std::string &sql_statement);
    virtual ~TDengineStatement();

    virtual void sub_exec() override;
    virtual bool sub_moveNext() override;
    virtual uint64_t sub_getLastRowid() override {
        return 0;
    }

    virtual void sub_bindNull(int idx) override {}
    virtual void sub_bindInt(int idx, int64_t value) override {}
    virtual void sub_bindDouble(int idx, double item) override {}
    virtual void sub_bindDatetime(int idx, const Datetime &item) override {}
    virtual void sub_bindText(int idx, const std::string &item) override {}
    virtual void sub_bindText(int idx, const char *item, size_t len) override {}
    virtual void sub_bindBlob(int idx, const std::string &item) override {}
    virtual void sub_bindBlob(int idx, const std::vector<char> &item) override {}

    virtual int sub_getNumColumns() const override;
    virtual void sub_getColumnAsInt64(int idx, int64_t &item) override;
    virtual void sub_getColumnAsDouble(int idx, double &item) override;
    virtual void sub_getColumnAsDatetime(int idx, Datetime &item) override;
    virtual void sub_getColumnAsText(int idx, std::string &item) override;
    virtual void sub_getColumnAsBlob(int idx, std::string &item) override;
    virtual void sub_getColumnAsBlob(int idx, std::vector<char> &item) override;

private:
    TAOS *m_taos{nullptr};

    TAOS_RES *m_query_result{nullptr};
    int m_num_fields{0};
    TAOS_FIELD *m_fields{nullptr};
    TAOS_ROW m_row;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_TDENGINE_STATEMENT_H */