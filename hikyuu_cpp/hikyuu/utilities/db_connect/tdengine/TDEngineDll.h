/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-11
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_SRC_UTILITIES_DB_CONNECT_TDENGINE_TDEngineDll_H_
#define HKU_SRC_UTILITIES_DB_CONNECT_TDENGINE_TDEngineDll_H_

#include <atomic>
#include "taos.h"
#include "hikyuu/utilities/DllLoader.h"

namespace hku {

class TDEngineDll final {
public:
    TDEngineDll();
    ~TDEngineDll();

    static TDEngineDll &instance();

    static int taos_init(void);
    static void taos_cleanup(void);

    static const char *taos_get_client_info();
    static const char *taos_get_server_info(TAOS *taos);

    static int taos_errno(TAOS_RES *res);
    static const char *taos_errstr(TAOS_RES *res);

    static TAOS *taos_connect(const char *ip, const char *user, const char *pass, const char *db,
                              uint16_t port);
    static void taos_close(TAOS *taos);

    static TAOS_RES *taos_query(TAOS *taos, const char *sql);

    static int taos_num_fields(TAOS_RES *res);
    static int taos_affected_rows(TAOS_RES *res);
    static TAOS_FIELD *taos_fetch_fields(TAOS_RES *res);
    static TAOS_ROW taos_fetch_row(TAOS_RES *res);

    static void taos_free_result(TAOS_RES *res);

    static TAOS_STMT *taos_stmt_init(TAOS *taos);
    static int taos_stmt_close(TAOS_STMT *stmt);
    static int taos_stmt_prepare(TAOS_STMT *stmt, const char *sql, unsigned long length);
    static int taos_stmt_is_insert(TAOS_STMT *stmt, int *insert);
    static int taos_stmt_num_params(TAOS_STMT *stmt, int *nums);
    static int taos_stmt_bind_param(TAOS_STMT *stmt, TAOS_MULTI_BIND *bind);
    static int taos_stmt_add_batch(TAOS_STMT *stmt);
    static int taos_stmt_execute(TAOS_STMT *stmt);
    static TAOS_RES *taos_stmt_use_result(TAOS_STMT *stmt);

    static char *taos_stmt_errstr(TAOS_STMT *stmt);

private:
    using taos_init_func = int (*)(void);
    taos_init_func m_taos_init{nullptr};

    using taos_cleanup_func = void (*)(void);
    taos_cleanup_func m_taos_cleanup{nullptr};

    using taos_get_client_info_func = const char *(*)(void);
    taos_get_client_info_func m_taos_get_client_info{nullptr};

    using taos_get_server_info_func = const char *(*)(TAOS * taos);
    taos_get_server_info_func m_taos_get_server_info{nullptr};

    using taos_errno_func = int (*)(TAOS_RES *res);
    taos_errno_func m_taos_errno{nullptr};

    using taos_errstr_func = const char *(*)(TAOS_RES * res);
    taos_errstr_func m_taos_errstr{nullptr};

    using taos_connect_func = TAOS *(*)(const char *ip, const char *user, const char *pass,
                                        const char *db, uint16_t port);
    taos_connect_func m_taos_connect{nullptr};

    using taos_close_func = void (*)(TAOS *taos);
    taos_close_func m_taos_close{nullptr};

    using taos_query_func = TAOS_RES *(*)(TAOS * taos, const char *sql);
    taos_query_func m_taos_query{nullptr};

    using taos_num_fields_func = int (*)(TAOS_RES *res);
    taos_num_fields_func m_taos_num_fields{nullptr};

    using taos_affected_rows_func = int (*)(TAOS_RES *res);
    taos_affected_rows_func m_taos_affected_rows{nullptr};

    using taos_fetch_fields_func = TAOS_FIELD *(*)(TAOS_RES * res);
    taos_fetch_fields_func m_taos_fetch_fields{nullptr};

    using taos_fetch_row_func = TAOS_ROW (*)(TAOS_RES *res);
    taos_fetch_row_func m_taos_fetch_row{nullptr};

    using taos_free_result_func = void (*)(TAOS_RES *res);
    taos_free_result_func m_taos_free_result{nullptr};

    using taos_stmt_init_func = TAOS_STMT *(*)(TAOS * taos);
    taos_stmt_init_func m_taos_stmt_init{nullptr};

    using taos_stmt_close_func = int (*)(TAOS_STMT *stmt);
    taos_stmt_close_func m_taos_stmt_close{nullptr};

    using taos_stmt_prepare_func = int (*)(TAOS_STMT *stmt, const char *sql, unsigned long length);
    taos_stmt_prepare_func m_taos_stmt_prepare{nullptr};

    using taos_stmt_is_insert_func = int (*)(TAOS_STMT *stmt, int *insert);
    taos_stmt_is_insert_func m_taos_stmt_is_insert{nullptr};

    using taos_stmt_num_params_func = int (*)(TAOS_STMT *stmt, int *nums);
    taos_stmt_num_params_func m_taos_stmt_num_params{nullptr};

    using taos_stmt_bind_param_func = int (*)(TAOS_STMT *stmt, TAOS_MULTI_BIND *bind);
    taos_stmt_bind_param_func m_taos_stmt_bind_param{nullptr};

    using taos_stmt_add_batch_func = int (*)(TAOS_STMT *stmt);
    taos_stmt_add_batch_func m_taos_stmt_add_batch{nullptr};

    using taos_stmt_execute_func = int (*)(TAOS_STMT *stmt);
    taos_stmt_execute_func m_taos_stmt_execute{nullptr};

    using taos_stmt_use_result_func = TAOS_RES *(*)(TAOS_STMT * stmt);
    taos_stmt_use_result_func m_taos_stmt_use_result{nullptr};

    using taos_stmt_errstr_func = char *(*)(TAOS_STMT * stmt);
    taos_stmt_errstr_func m_taos_stmt_errstr{nullptr};

private:
    DllLoader m_loader;
    std::atomic_bool m_loaded{false};
};

}  // namespace hku

#endif /* HKU_SRC_UTILITIES_DB_CONNECT_TDENGINE_TDEngineDll_H_ */