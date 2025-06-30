/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-30
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_DB_CONNECT_TAOS_MACRO_H
#define HIKYUU_DB_CONNECT_TAOS_MACRO_H

#include "hikyuu/utilities/db_connect/DBConnectBase.h"
#include "hikyuu/utilities/db_connect/SQLStatementBase.h"

namespace hku {

#define TAOS_BIND0(TableT, table, key)           \
public:                                          \
    TableT() = default;                          \
    TableT(const TableT&) = default;             \
    TableT& operator=(const TableT&) = default;  \
    TableT(TableT&& rv) : m_id(rv.m_id) {        \
        rv.m_id = 0;                             \
    }                                            \
    TableT& operator=(TableT&& rv) {             \
        if (this == &rv)                         \
            return *this;                        \
        m_id = rv.m_id;                          \
        rv.m_id = 0;                             \
        return *this;                            \
    }                                            \
                                                 \
private:                                         \
    uint64_t m_id = 0;                           \
                                                 \
public:                                          \
    bool valid() const {                         \
        return m_id != 0;                        \
    }                                            \
    uint64_t id() const {                        \
        return m_id;                             \
    }                                            \
    void id(uint64_t id) {                       \
        m_id = id;                               \
    }                                            \
    uint64_t rowid() const {                     \
        return m_id;                             \
    }                                            \
    void rowid(uint64_t id) {                    \
        m_id = id;                               \
    }                                            \
    Datetime ts() const {                        \
        return Datetime::fromTimestampUTC(m_id); \
    }                                            \
    static std::string getTableName() {          \
        return #table;                           \
    }                                            \
    static const char* getSelectSQL() {          \
        return "select `" #key "` from " #table; \
    }                                            \
    void load(const SQLStatementPtr& st) {       \
        st->getColumn(0, m_id);                  \
    }

#define TAOS_BIND1(TableT, table, key, f1)                      \
public:                                                         \
    TableT() = default;                                         \
    TableT(const TableT&) = default;                            \
    TableT& operator=(const TableT&) = default;                 \
    TableT(TableT&& rv) : m_id(rv.m_id), f1(std::move(rv.f1)) { \
        rv.m_id = 0;                                            \
    }                                                           \
    TableT& operator=(TableT&& rv) {                            \
        if (this == &rv)                                        \
            return *this;                                       \
        m_id = rv.m_id;                                         \
        f1 = std::move(rv.f1);                                  \
        rv.m_id = 0;                                            \
        return *this;                                           \
    }                                                           \
                                                                \
private:                                                        \
    uint64_t m_id = 0;                                          \
                                                                \
public:                                                         \
    bool valid() const {                                        \
        return m_id != 0;                                       \
    }                                                           \
    uint64_t id() const {                                       \
        return m_id;                                            \
    }                                                           \
    void id(uint64_t id) {                                      \
        m_id = id;                                              \
    }                                                           \
    uint64_t rowid() const {                                    \
        return m_id;                                            \
    }                                                           \
    void rowid(uint64_t id) {                                   \
        m_id = id;                                              \
    }                                                           \
    Datetime ts() const {                                       \
        return Datetime::fromTimestampUTC(m_id);                \
    }                                                           \
    static std::string getTableName() {                         \
        return #table;                                          \
    }                                                           \
    static const char* getSelectSQL() {                         \
        return "select `" #key "`,`" #f1 "` from " #table;      \
    }                                                           \
    void load(const SQLStatementPtr& st) {                      \
        st->getColumn(0, m_id, f1);                             \
    }

#define TAOS_BIND2(TableT, table, key, f1, f2)                                        \
public:                                                                               \
    TableT() = default;                                                               \
    TableT(const TableT&) = default;                                                  \
    TableT& operator=(const TableT&) = default;                                       \
    TableT(TableT&& rv) : m_id(rv.m_id), f1(std::move(rv.f1)), f2(std::move(rv.f2)) { \
        rv.m_id = 0;                                                                  \
    }                                                                                 \
    TableT& operator=(TableT&& rv) {                                                  \
        if (this == &rv)                                                              \
            return *this;                                                             \
        m_id = rv.m_id;                                                               \
        f1 = std::move(rv.f1);                                                        \
        f2 = std::move(rv.f2);                                                        \
        rv.m_id = 0;                                                                  \
        return *this;                                                                 \
    }                                                                                 \
                                                                                      \
private:                                                                              \
    uint64_t m_id = 0;                                                                \
                                                                                      \
public:                                                                               \
    bool valid() const {                                                              \
        return m_id != 0;                                                             \
    }                                                                                 \
    uint64_t id() const {                                                             \
        return m_id;                                                                  \
    }                                                                                 \
    void id(uint64_t id) {                                                            \
        m_id = id;                                                                    \
    }                                                                                 \
    uint64_t rowid() const {                                                          \
        return m_id;                                                                  \
    }                                                                                 \
    void rowid(uint64_t id) {                                                         \
        m_id = id;                                                                    \
    }                                                                                 \
    Datetime ts() const {                                                             \
        return Datetime::fromTimestampUTC(m_id);                                      \
    }                                                                                 \
    static std::string getTableName() {                                               \
        return #table;                                                                \
    }                                                                                 \
    static const char* getSelectSQL() {                                               \
        return "select `" #key "`,`" #f1 "`,`" #f2 "` from " #table;                  \
    }                                                                                 \
    void load(const SQLStatementPtr& st) {                                            \
        st->getColumn(0, m_id, f1, f2);                                               \
    }

#define TAOS_BIND3(TableT, table, key, f1, f2, f3)                                      \
public:                                                                                 \
    TableT() = default;                                                                 \
    TableT(const TableT&) = default;                                                    \
    TableT& operator=(const TableT&) = default;                                         \
    TableT(TableT&& rv)                                                                 \
    : m_id(rv.m_id), f1(std::move(rv.f1)), f2(std::move(rv.f2)), f3(std::move(rv.f3)) { \
        rv.m_id = 0;                                                                    \
    }                                                                                   \
    TableT& operator=(TableT&& rv) {                                                    \
        if (this == &rv)                                                                \
            return *this;                                                               \
        m_id = rv.m_id;                                                                 \
        f1 = std::move(rv.f1);                                                          \
        f2 = std::move(rv.f2);                                                          \
        f3 = std::move(rv.f3);                                                          \
        rv.m_id = 0;                                                                    \
        return *this;                                                                   \
    }                                                                                   \
                                                                                        \
private:                                                                                \
    uint64_t m_id = 0;                                                                  \
                                                                                        \
public:                                                                                 \
    bool valid() const {                                                                \
        return m_id != 0;                                                               \
    }                                                                                   \
    uint64_t id() const {                                                               \
        return m_id;                                                                    \
    }                                                                                   \
    void id(uint64_t id) {                                                              \
        m_id = id;                                                                      \
    }                                                                                   \
    uint64_t rowid() const {                                                            \
        return m_id;                                                                    \
    }                                                                                   \
    void rowid(uint64_t id) {                                                           \
        m_id = id;                                                                      \
    }                                                                                   \
    Datetime ts() const {                                                               \
        return Datetime::fromTimestampUTC(m_id);                                        \
    }                                                                                   \
    static std::string getTableName() {                                                 \
        return #table;                                                                  \
    }                                                                                   \
    static const char* getSelectSQL() {                                                 \
        return "select `" #key "`,`" #f1 "`,`" #f2 "`,`" #f3 "` from " #table;          \
    }                                                                                   \
    void load(const SQLStatementPtr& st) {                                              \
        st->getColumn(0, m_id, f1, f2, f3);                                             \
    }

#define TAOS_BIND4(TableT, table, key, f1, f2, f3, f4)                                   \
public:                                                                                  \
    TableT() = default;                                                                  \
    TableT(const TableT&) = default;                                                     \
    TableT& operator=(const TableT&) = default;                                          \
    TableT(TableT&& rv)                                                                  \
    : m_id(rv.m_id),                                                                     \
      f1(std::move(rv.f1)),                                                              \
      f2(std::move(rv.f2)),                                                              \
      f3(std::move(rv.f3)),                                                              \
      f4(std::move(rv.f4)) {                                                             \
        rv.m_id = 0;                                                                     \
    }                                                                                    \
    TableT& operator=(TableT&& rv) {                                                     \
        if (this == &rv)                                                                 \
            return *this;                                                                \
        m_id = rv.m_id;                                                                  \
        f1 = std::move(rv.f1);                                                           \
        f2 = std::move(rv.f2);                                                           \
        f3 = std::move(rv.f3);                                                           \
        f4 = std::move(rv.f4);                                                           \
        rv.m_id = 0;                                                                     \
        return *this;                                                                    \
    }                                                                                    \
                                                                                         \
private:                                                                                 \
    uint64_t m_id = 0;                                                                   \
                                                                                         \
public:                                                                                  \
    bool valid() const {                                                                 \
        return m_id != 0;                                                                \
    }                                                                                    \
    uint64_t id() const {                                                                \
        return m_id;                                                                     \
    }                                                                                    \
    void id(uint64_t id) {                                                               \
        m_id = id;                                                                       \
    }                                                                                    \
    uint64_t rowid() const {                                                             \
        return m_id;                                                                     \
    }                                                                                    \
    void rowid(uint64_t id) {                                                            \
        m_id = id;                                                                       \
    }                                                                                    \
    Datetime ts() const {                                                                \
        return Datetime::fromTimestampUTC(m_id);                                         \
    }                                                                                    \
    static std::string getTableName() {                                                  \
        return #table;                                                                   \
    }                                                                                    \
    static const char* getSelectSQL() {                                                  \
        return "select `" #key "`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "` from " #table; \
    }                                                                                    \
    void load(const SQLStatementPtr& st) {                                               \
        st->getColumn(0, m_id, f1, f2, f3, f4);                                          \
    }

#define TAOS_BIND5(TableT, table, key, f1, f2, f3, f4, f5)                                         \
public:                                                                                            \
    TableT() = default;                                                                            \
    TableT(const TableT&) = default;                                                               \
    TableT& operator=(const TableT&) = default;                                                    \
    TableT(TableT&& rv)                                                                            \
    : m_id(rv.m_id),                                                                               \
      f1(std::move(rv.f1)),                                                                        \
      f2(std::move(rv.f2)),                                                                        \
      f3(std::move(rv.f3)),                                                                        \
      f4(std::move(rv.f4)),                                                                        \
      f5(std::move(rv.f5)) {                                                                       \
        rv.m_id = 0;                                                                               \
    }                                                                                              \
    TableT& operator=(TableT&& rv) {                                                               \
        if (this == &rv)                                                                           \
            return *this;                                                                          \
        m_id = rv.m_id;                                                                            \
        f1 = std::move(rv.f1);                                                                     \
        f2 = std::move(rv.f2);                                                                     \
        f3 = std::move(rv.f3);                                                                     \
        f4 = std::move(rv.f4);                                                                     \
        f5 = std::move(rv.f5);                                                                     \
        rv.m_id = 0;                                                                               \
        return *this;                                                                              \
    }                                                                                              \
                                                                                                   \
private:                                                                                           \
    uint64_t m_id = 0;                                                                             \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_id != 0;                                                                          \
    }                                                                                              \
    uint64_t id() const {                                                                          \
        return m_id;                                                                               \
    }                                                                                              \
    void id(uint64_t id) {                                                                         \
        m_id = id;                                                                                 \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_id;                                                                               \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_id = id;                                                                                 \
    }                                                                                              \
    Datetime ts() const {                                                                          \
        return Datetime::fromTimestampUTC(m_id);                                                   \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #key "`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "` from " #table; \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5);                                                \
    }

#define TAOS_BIND6(TableT, table, key, f1, f2, f3, f4, f5, f6)                             \
public:                                                                                    \
    TableT() = default;                                                                    \
    TableT(const TableT&) = default;                                                       \
    TableT& operator=(const TableT&) = default;                                            \
    TableT(TableT&& rv)                                                                    \
    : m_id(rv.m_id),                                                                       \
      f1(std::move(rv.f1)),                                                                \
      f2(std::move(rv.f2)),                                                                \
      f3(std::move(rv.f3)),                                                                \
      f4(std::move(rv.f4)),                                                                \
      f5(std::move(rv.f5)),                                                                \
      f6(std::move(rv.f6)) {                                                               \
        rv.m_id = 0;                                                                       \
    }                                                                                      \
    TableT& operator=(TableT&& rv) {                                                       \
        if (this == &rv)                                                                   \
            return *this;                                                                  \
        m_id = rv.m_id;                                                                    \
        f1 = std::move(rv.f1);                                                             \
        f2 = std::move(rv.f2);                                                             \
        f3 = std::move(rv.f3);                                                             \
        f4 = std::move(rv.f4);                                                             \
        f5 = std::move(rv.f5);                                                             \
        f6 = std::move(rv.f6);                                                             \
        rv.m_id = 0;                                                                       \
        return *this;                                                                      \
    }                                                                                      \
                                                                                           \
private:                                                                                   \
    uint64_t m_id = 0;                                                                     \
                                                                                           \
public:                                                                                    \
    bool valid() const {                                                                   \
        return m_id != 0;                                                                  \
    }                                                                                      \
    uint64_t id() const {                                                                  \
        return m_id;                                                                       \
    }                                                                                      \
    void id(uint64_t id) {                                                                 \
        m_id = id;                                                                         \
    }                                                                                      \
    uint64_t rowid() const {                                                               \
        return m_id;                                                                       \
    }                                                                                      \
    void rowid(uint64_t id) {                                                              \
        m_id = id;                                                                         \
    }                                                                                      \
    Datetime ts() const {                                                                  \
        return Datetime::fromTimestampUTC(m_id);                                           \
    }                                                                                      \
    static std::string getTableName() {                                                    \
        return #table;                                                                     \
    }                                                                                      \
    static const char* getSelectSQL() {                                                    \
        return "select `" #key "`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "` from " #table;                                                           \
    }                                                                                      \
    void load(const SQLStatementPtr& st) {                                                 \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6);                                    \
    }

#define TAOS_BIND7(TableT, table, key, f1, f2, f3, f4, f5, f6, f7)                         \
public:                                                                                    \
    TableT() = default;                                                                    \
    TableT(const TableT&) = default;                                                       \
    TableT& operator=(const TableT&) = default;                                            \
    TableT(TableT&& rv)                                                                    \
    : m_id(rv.m_id),                                                                       \
      f1(std::move(rv.f1)),                                                                \
      f2(std::move(rv.f2)),                                                                \
      f3(std::move(rv.f3)),                                                                \
      f4(std::move(rv.f4)),                                                                \
      f5(std::move(rv.f5)),                                                                \
      f6(std::move(rv.f6)),                                                                \
      f7(std::move(rv.f7)) {                                                               \
        rv.m_id = 0;                                                                       \
    }                                                                                      \
    TableT& operator=(TableT&& rv) {                                                       \
        if (this == &rv)                                                                   \
            return *this;                                                                  \
        m_id = rv.m_id;                                                                    \
        f1 = std::move(rv.f1);                                                             \
        f2 = std::move(rv.f2);                                                             \
        f3 = std::move(rv.f3);                                                             \
        f4 = std::move(rv.f4);                                                             \
        f5 = std::move(rv.f5);                                                             \
        f6 = std::move(rv.f6);                                                             \
        f7 = std::move(rv.f7);                                                             \
        rv.m_id = 0;                                                                       \
        return *this;                                                                      \
    }                                                                                      \
                                                                                           \
private:                                                                                   \
    uint64_t m_id = 0;                                                                     \
                                                                                           \
public:                                                                                    \
    bool isValid() const {                                                                 \
        return m_id != 0;                                                                  \
    }                                                                                      \
    uint64_t id() const {                                                                  \
        return m_id;                                                                       \
    }                                                                                      \
    void id(uint64_t id) {                                                                 \
        m_id = id;                                                                         \
    }                                                                                      \
    uint64_t rowid() const {                                                               \
        return m_id;                                                                       \
    }                                                                                      \
    void rowid(uint64_t id) {                                                              \
        m_id = id;                                                                         \
    }                                                                                      \
    Datetime ts() const {                                                                  \
        return Datetime::fromTimestampUTC(m_id);                                           \
    }                                                                                      \
    static std::string getTableName() {                                                    \
        return #table;                                                                     \
    }                                                                                      \
    static const char* getSelectSQL() {                                                    \
        return "select `" #key "`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "` from " #table;                                                 \
    }                                                                                      \
    void load(const SQLStatementPtr& st) {                                                 \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7);                                \
    }

} /* namespace hku */

#endif /* HIKYUU_DB_CONNECT_TAOS_MACRO_H */
