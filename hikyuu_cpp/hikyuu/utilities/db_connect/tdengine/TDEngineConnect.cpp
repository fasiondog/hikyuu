/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-04
 *      Author: fasiondog
 */

#include "hikyuu/utilities/DllLoader.h"
#include "TDEngineDll.h"
#include "TDengineConnect.h"

namespace hku {

TDengineConnect::TDengineConnect(const Parameter& param) : DBConnectBase(param) {
    close();
    connect();
}

TDengineConnect::~TDengineConnect() {
    close();
}

void TDengineConnect::close() {
    if (m_taos) {
        TDEngineDll::taos_close(m_taos);
        m_taos = nullptr;
    }
}

SQLStatementPtr TDengineConnect::getStatement(const std::string& sql_statement) {
    return std::make_shared<TDengineStatement>(this, sql_statement);
}

void TDengineConnect::connect() {
    try {
        std::string host = tryGetParam<std::string>("host", "localhost");
        std::string usr = tryGetParam<std::string>("usr", "root");
        std::string pwd = tryGetParam<std::string>("pwd", "taosdata");
        std::string database = tryGetParam<std::string>("db", "");
        uint16_t port = tryGetParam<int>("port", 6030);

        m_taos =
          TDEngineDll::taos_connect(host.c_str(), usr.c_str(), pwd.c_str(), database.c_str(), port);
        HKU_CHECK(m_taos, "Failed to connect to {}:{}, ErrCode: 0x{:x}, ErrMessage: {}.", host,
                  port, TDEngineDll::taos_errno(nullptr), TDEngineDll::taos_errstr(nullptr));

    } catch (const hku::exception& e) {
        close();
        HKU_ERROR(e.what());
        HKU_THROW("Failed create TDengineConnect! {}", e.what());

    } catch (const std::exception& e) {
        close();
        HKU_ERROR(e.what());
        HKU_THROW("Failed create TDengineConnect instance! {}", e.what());

    } catch (...) {
        close();
        const char* errmsg = "Failed create TDengineConnect instance! Unknown error";
        HKU_ERROR(errmsg);
        HKU_THROW("{}", errmsg);
    }
}

int64_t TDengineConnect::exec(const std::string& sql_string) {
#if HKU_SQL_TRACE
    HKU_DEBUG(sql_string);
#endif
    HKU_CHECK(m_taos || reconnect(), "Failed to connect to TDengine server! {}", sql_string);

    TAOS_RES* result = TDEngineDll::taos_query(m_taos, sql_string.c_str());
    int code = TDEngineDll::taos_errno(result);
    if (code != 0) {
        // 尝试检查连接状态，连接状态失败时尝试重连，重连失败抛出异常
        if (!ping() && !reconnect()) {
            HKU_THROW("Failed reconnect to TDengine server! {}", sql_string);
        }

        // 重连后重新执行
        result = TDEngineDll::taos_query(m_taos, sql_string.c_str());
        code = TDEngineDll::taos_errno(result);
    }

    SQL_CHECK(code == 0, code, "SQL error: {}! ErrCode: 0x{:x}, ErrMessage: {}", sql_string, code,
              TDEngineDll::taos_errstr(result));

    int rows = TDEngineDll::taos_affected_rows(result);
    TDEngineDll::taos_free_result(result);
    return rows;
}

bool TDengineConnect::reconnect() noexcept {
    try {
        close();
        connect();
        return true;
    } catch (...) {
        return false;
    }
}

bool TDengineConnect::ping() {
    HKU_IF_RETURN(m_taos == nullptr, false);

    TAOS_RES* res = TDEngineDll::taos_query(m_taos, "SELECT 1");
    HKU_IF_RETURN(res == nullptr, false);

    TDEngineDll::taos_free_result(res);
    return true;
}

bool TDengineConnect::tableExist(const std::string& tablename) {
    bool result = false;
    try {
        SQLStatementPtr st = getStatement(fmt::format("SELECT 1 FROM {} LIMIT 1;", tablename));
        st->exec();
        result = true;
    } catch (...) {
        result = false;
    }
    return result;
}

}  // namespace hku