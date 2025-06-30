/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-11
 *      Author: fasiondog
 */

#include "TDEngineDll.h"

namespace hku {

TDEngineDll::TDEngineDll() {
    m_loaded = m_loader.load("taos");

    m_taos_init = reinterpret_cast<taos_init_func>(m_loader.getSymbol("taos_init"));
    HKU_ASSERT(m_taos_init);
    if (m_taos_init() != 0) {
        fmt::print("[TDEngineDll] taos_init failed\n");
    }

    m_taos_cleanup = reinterpret_cast<taos_cleanup_func>(m_loader.getSymbol("taos_cleanup"));
    HKU_ASSERT(m_taos_cleanup);

    m_taos_get_client_info =
      reinterpret_cast<taos_get_client_info_func>(m_loader.getSymbol("taos_get_client_info"));
    HKU_ASSERT(m_taos_get_client_info);

    m_taos_get_server_info =
      reinterpret_cast<taos_get_server_info_func>(m_loader.getSymbol("taos_get_server_info"));
    HKU_ASSERT(m_taos_get_server_info);

    m_taos_errno = reinterpret_cast<taos_errno_func>(m_loader.getSymbol("taos_errno"));
    HKU_ASSERT(m_taos_errno);

    m_taos_errstr = reinterpret_cast<taos_errstr_func>(m_loader.getSymbol("taos_errstr"));
    HKU_ASSERT(m_taos_errstr);

    m_taos_connect = reinterpret_cast<taos_connect_func>(m_loader.getSymbol("taos_connect"));
    HKU_ASSERT(m_taos_connect);

    m_taos_close = reinterpret_cast<taos_close_func>(m_loader.getSymbol("taos_close"));
    HKU_ASSERT(m_taos_close);

    m_taos_query = reinterpret_cast<taos_query_func>(m_loader.getSymbol("taos_query"));
    HKU_ASSERT(m_taos_query);

    m_taos_num_fields =
      reinterpret_cast<taos_num_fields_func>(m_loader.getSymbol("taos_num_fields"));
    HKU_ASSERT(m_taos_num_fields);

    m_taos_affected_rows =
      reinterpret_cast<taos_affected_rows_func>(m_loader.getSymbol("taos_affected_rows"));
    HKU_ASSERT(m_taos_affected_rows);

    m_taos_fetch_fields =
      reinterpret_cast<taos_fetch_fields_func>(m_loader.getSymbol("taos_fetch_fields"));
    HKU_ASSERT(m_taos_fetch_fields);

    m_taos_fetch_row = reinterpret_cast<taos_fetch_row_func>(m_loader.getSymbol("taos_fetch_row"));
    HKU_ASSERT(m_taos_fetch_row);

    m_taos_free_result =
      reinterpret_cast<taos_free_result_func>(m_loader.getSymbol("taos_free_result"));
    HKU_ASSERT(m_taos_free_result);

    m_taos_stmt_init = reinterpret_cast<taos_stmt_init_func>(m_loader.getSymbol("taos_stmt_init"));
    HKU_ASSERT(m_taos_stmt_init);

    m_taos_stmt_close =
      reinterpret_cast<taos_stmt_close_func>(m_loader.getSymbol("taos_stmt_close"));
    HKU_ASSERT(m_taos_stmt_close);

    m_taos_stmt_prepare =
      reinterpret_cast<taos_stmt_prepare_func>(m_loader.getSymbol("taos_stmt_prepare"));
    HKU_ASSERT(m_taos_stmt_prepare);

    m_taos_stmt_is_insert =
      reinterpret_cast<taos_stmt_is_insert_func>(m_loader.getSymbol("taos_stmt_is_insert"));
    HKU_ASSERT(m_taos_stmt_is_insert);

    m_taos_stmt_num_params =
      reinterpret_cast<taos_stmt_num_params_func>(m_loader.getSymbol("taos_stmt_num_params"));
    HKU_ASSERT(m_taos_stmt_num_params);

    m_taos_stmt_bind_param =
      reinterpret_cast<taos_stmt_bind_param_func>(m_loader.getSymbol("taos_stmt_bind_param"));
    HKU_ASSERT(m_taos_stmt_bind_param);

    m_taos_stmt_add_batch =
      reinterpret_cast<taos_stmt_add_batch_func>(m_loader.getSymbol("taos_stmt_add_batch"));
    HKU_ASSERT(m_taos_stmt_add_batch);

    m_taos_stmt_execute =
      reinterpret_cast<taos_stmt_execute_func>(m_loader.getSymbol("taos_stmt_execute"));
    HKU_ASSERT(m_taos_stmt_execute);

    m_taos_stmt_use_result =
      reinterpret_cast<taos_stmt_use_result_func>(m_loader.getSymbol("taos_stmt_use_result"));
    HKU_ASSERT(m_taos_stmt_use_result);

    m_taos_stmt_errstr =
      reinterpret_cast<taos_stmt_errstr_func>(m_loader.getSymbol("taos_stmt_errstr"));
    HKU_ASSERT(m_taos_stmt_errstr);
}

TDEngineDll::~TDEngineDll() {
    if (m_taos_cleanup) {
        // fmt::print("[TDEngineDll] taos_cleanup\n");
        instance().m_taos_cleanup();
    }
    m_loader.unload();
}

TDEngineDll &TDEngineDll::instance() {
    static TDEngineDll instance;
    return instance;
}

int TDEngineDll::taos_init(void) {
    return TDEngineDll::instance().m_taos_init();
}

void TDEngineDll::taos_cleanup(void) {
    TDEngineDll::instance().m_taos_cleanup();
}

const char *TDEngineDll::taos_get_client_info() {
    return TDEngineDll::instance().m_taos_get_client_info();
}

const char *TDEngineDll::taos_get_server_info(TAOS *taos) {
    return TDEngineDll::instance().m_taos_get_server_info(taos);
}

int TDEngineDll::taos_errno(TAOS_RES *res) {
    return TDEngineDll::instance().m_taos_errno(res);
}

const char *TDEngineDll::taos_errstr(TAOS_RES *res) {
    return TDEngineDll::instance().m_taos_errstr(res);
}

TAOS *TDEngineDll::taos_connect(const char *ip, const char *user, const char *pass, const char *db,
                                uint16_t port) {
    return TDEngineDll::instance().m_taos_connect(ip, user, pass, db, port);
}

void TDEngineDll::taos_close(TAOS *taos) {
    TDEngineDll::instance().m_taos_close(taos);
}

TAOS_RES *TDEngineDll::taos_query(TAOS *taos, const char *sql) {
    return TDEngineDll::instance().m_taos_query(taos, sql);
}

int TDEngineDll::taos_num_fields(TAOS_RES *res) {
    return TDEngineDll::instance().m_taos_num_fields(res);
}

int TDEngineDll::taos_affected_rows(TAOS_RES *res) {
    return TDEngineDll::instance().m_taos_affected_rows(res);
}

TAOS_FIELD *TDEngineDll::taos_fetch_fields(TAOS_RES *res) {
    return TDEngineDll::instance().m_taos_fetch_fields(res);
}

TAOS_ROW TDEngineDll::taos_fetch_row(TAOS_RES *res) {
    return TDEngineDll::instance().m_taos_fetch_row(res);
}

void TDEngineDll::taos_free_result(TAOS_RES *res) {
    TDEngineDll::instance().m_taos_free_result(res);
}

TAOS_STMT *TDEngineDll::taos_stmt_init(TAOS *taos) {
    return TDEngineDll::instance().m_taos_stmt_init(taos);
}

int TDEngineDll::taos_stmt_close(TAOS_STMT *stmt) {
    return TDEngineDll::instance().m_taos_stmt_close(stmt);
}

int TDEngineDll::taos_stmt_prepare(TAOS_STMT *stmt, const char *sql, unsigned long length) {
    return TDEngineDll::instance().m_taos_stmt_prepare(stmt, sql, length);
}

int TDEngineDll::taos_stmt_is_insert(TAOS_STMT *stmt, int *insert) {
    return TDEngineDll::instance().m_taos_stmt_is_insert(stmt, insert);
}

int TDEngineDll::taos_stmt_num_params(TAOS_STMT *stmt, int *nums) {
    return TDEngineDll::instance().m_taos_stmt_num_params(stmt, nums);
}

int TDEngineDll::taos_stmt_bind_param(TAOS_STMT *stmt, TAOS_MULTI_BIND *bind) {
    return TDEngineDll::instance().m_taos_stmt_bind_param(stmt, bind);
}

int TDEngineDll::taos_stmt_add_batch(TAOS_STMT *stmt) {
    return TDEngineDll::instance().m_taos_stmt_add_batch(stmt);
}

int TDEngineDll::taos_stmt_execute(TAOS_STMT *stmt) {
    return TDEngineDll::instance().m_taos_stmt_execute(stmt);
}

TAOS_RES *TDEngineDll::taos_stmt_use_result(TAOS_STMT *stmt) {
    return TDEngineDll::instance().m_taos_stmt_use_result(stmt);
}

char *TDEngineDll::taos_stmt_errstr(TAOS_STMT *stmt) {
    return TDEngineDll::instance().m_taos_stmt_errstr(stmt);
}

}  // namespace hku