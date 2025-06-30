/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-04
 *      Author: fasiondog
 */

#include "TDEngineDll.h"
#include "TDengineConnect.h"
#include "TDengineStatement.h"

namespace hku {

char g_taos_is_null = 1;

TDengineStatement::TDengineStatement(DBConnectBase *driver, const std::string &sql_statement)
: SQLStatementBase(driver, sql_statement) {
    const TDengineConnect *connect = dynamic_cast<TDengineConnect *>(driver);
    HKU_CHECK(connect, "Failed create statement: {}! Failed dynamic_cast<TDengineConnect*>!",
              sql_statement);

    m_taos = connect->getRawTAOS();

    auto sql = sql_statement;
    to_lower(sql);

    auto pos = sql.find("insert");
    HKU_CHECK(pos == std::string::npos, "Not support insert statement: {}!", sql_statement);

    pos = sql.find("update");
    HKU_CHECK(pos == std::string::npos, "Not support update statement: {}!", sql_statement);
}

TDengineStatement::~TDengineStatement() {
    if (m_query_result) {
        TDEngineDll::taos_free_result(m_query_result);
    }
}

void TDengineStatement::sub_exec() {
    m_query_result = TDEngineDll::taos_query(m_taos, m_sql_string.c_str());
    int code = TDEngineDll::taos_errno(m_query_result);
    SQL_CHECK(m_query_result != nullptr && code == 0, TDEngineDll::taos_errno(m_query_result),
              "Failed taos_query: {}! {}", TDEngineDll::taos_errstr(m_query_result), m_sql_string);
}

bool TDengineStatement::sub_moveNext() {
    HKU_IF_RETURN(!m_query_result, false);
    m_num_fields = TDEngineDll::taos_num_fields(m_query_result);
    m_fields = TDEngineDll::taos_fetch_fields(m_query_result);
    m_row = TDEngineDll::taos_fetch_row(m_query_result);
    return m_row != nullptr;
}

int TDengineStatement::sub_getNumColumns() const {
    return m_num_fields;
}

void TDengineStatement::sub_getColumnAsInt64(int idx, int64_t &item) {
    HKU_CHECK(idx < m_num_fields, "idx out of range! idx: {}, total: {}", idx, m_num_fields);

    if (m_row[idx] == NULL) {
        item = 0;
        return;
    }

    try {
        switch (m_fields[idx].type) {
            case TSDB_DATA_TYPE_TIMESTAMP: {
                int64_t tmp = *((int64_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_TINYINT: {
                int8_t tmp = *((int8_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_SMALLINT: {
                int16_t tmp = *((int16_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_INT: {
                int32_t tmp = *((int32_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_BIGINT: {
                int64_t tmp = *((int64_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_UTINYINT: {
                uint8_t tmp = *((uint8_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_USMALLINT: {
                uint16_t tmp = *((uint16_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_UINT: {
                uint32_t tmp = *((uint32_t *)m_row[idx]);
                item = tmp;
                break;
            }
            case TSDB_DATA_TYPE_UBIGINT: {
                uint64_t tmp = *((uint64_t *)m_row[idx]);
                item = tmp;
                break;
            }

            default:
                HKU_THROW("Field type mismatch! idx: {}, type: {}", idx, m_fields[idx].type);
        }
    } catch (const hku::exception &) {
        throw;
    } catch (const std::exception &e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

void TDengineStatement::sub_getColumnAsDouble(int idx, double &item) {
    HKU_CHECK(idx < m_num_fields, "idx out of range! idx: {}, total: {}", idx, m_num_fields);

    if (m_row[idx] == NULL) {
        item = 0.0;
        return;
    }

    try {
        if (m_fields[idx].type == TSDB_DATA_TYPE_DOUBLE) {
            item = *((double *)m_row[idx]);
        } else if (m_fields[idx].type == TSDB_DATA_TYPE_FLOAT) {
            float tmp = *((float *)m_row[idx]);
            item = tmp;
        } else {
            HKU_THROW("Field type mismatch! idx: {}", idx);
        }

    } catch (const hku::exception &) {
        throw;
    } catch (const std::exception &e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

void TDengineStatement::sub_getColumnAsDatetime(int idx, Datetime &item) {
    HKU_CHECK(idx < m_num_fields, "idx out of range! idx: {}, total: {}", idx, m_num_fields);
    if (m_row[idx] == NULL) {
        item = Null<Datetime>();
        return;
    }

    try {
        if (m_fields[idx].type == TSDB_DATA_TYPE_TIMESTAMP) {
            int64_t tmp = *((int64_t *)m_row[idx]);
            item = Datetime::fromTimestamp(tmp);
        } else {
            HKU_THROW("Field type mismatch! idx: {}", idx);
        }

    } catch (const hku::exception &) {
        throw;
    } catch (const std::exception &e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

typedef uint16_t VarDataLenT;
#define varDataLen(v) ((VarDataLenT *)(v))[0]
#define VARSTR_HEADER_SIZE sizeof(VarDataLenT)

void TDengineStatement::sub_getColumnAsText(int idx, std::string &item) {
    HKU_CHECK(idx < m_num_fields, "idx out of range! idx: {}, total: {}", idx, m_num_fields);

    if (m_row[idx] == NULL) {
        item.clear();
        return;
    }

    try {
        int32_t charLen = varDataLen((char *)m_row[idx] - VARSTR_HEADER_SIZE);
        char *p = (char *)m_row[idx];
        std::ostringstream buf;
        for (int32_t i = 0; i < charLen; i++) {
            buf << p[i];
        }
        item = buf.str();

    } catch (const hku::exception &) {
        throw;
    } catch (const std::exception &e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

void TDengineStatement::sub_getColumnAsBlob(int idx, std::string &item) {
    sub_getColumnAsText(idx, item);
}

void TDengineStatement::sub_getColumnAsBlob(int idx, std::vector<char> &item) {
    HKU_CHECK(idx < m_num_fields, "idx out of range! idx: {}, total: {}", idx, m_num_fields);

    if (m_row[idx] == NULL) {
        item.clear();
        return;
    }

    try {
        int32_t charLen = varDataLen((char *)m_row[idx] - VARSTR_HEADER_SIZE);
        char *p = (char *)m_row[idx];
        item.resize(charLen);
        memcpy(item.data(), p, charLen);

    } catch (const hku::exception &) {
        throw;
    } catch (const std::exception &e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

}  // namespace hku