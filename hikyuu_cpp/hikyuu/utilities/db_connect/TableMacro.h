/*
 * TableMacro.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-14
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_TABLE_MACRO_H
#define HIKYUU_DB_CONNECT_TABLE_MACRO_H

#include "DBConnectBase.h"
#include "SQLStatementBase.h"

namespace hku {

#define TABLE_BIND0(TableT, table)              \
public:                                         \
    TableT() = default;                         \
    TableT(const TableT&) = default;            \
    TableT& operator=(const TableT&) = default; \
    TableT(TableT&& rv) : m_id(rv.m_id) {       \
        rv.m_id = 0;                            \
    }                                           \
    TableT& operator=(TableT&& rv) {            \
        if (this == &rv)                        \
            return *this;                       \
        m_id = rv.m_id;                         \
        rv.m_id = 0;                            \
        return *this;                           \
    }                                           \
                                                \
private:                                        \
    uint64_t m_id = 0;                          \
                                                \
public:                                         \
    bool valid() const {                        \
        return m_id != 0;                       \
    }                                           \
    uint64_t id() const {                       \
        return m_id;                            \
    }                                           \
    void id(uint64_t id) {                      \
        m_id = id;                              \
    }                                           \
    uint64_t rowid() const {                    \
        return m_id;                            \
    }                                           \
    void rowid(uint64_t id) {                   \
        m_id = id;                              \
    }                                           \
    static std::string getTableName() {         \
        return #table;                          \
    }                                           \
    static const char* getSelectSQL() {         \
        return "select `id` from `" #table "`"; \
    }                                           \
    void load(const SQLStatementPtr& st) {      \
        st->getColumn(0, m_id);                 \
    }

#define TABLE_BIND1(TableT, table, f1)                             \
public:                                                            \
    TableT() = default;                                            \
    TableT(const TableT&) = default;                               \
    TableT& operator=(const TableT&) = default;                    \
    TableT(TableT&& rv) : m_id(rv.m_id), f1(std::move(rv.f1)) {    \
        rv.m_id = 0;                                               \
    }                                                              \
    TableT& operator=(TableT&& rv) {                               \
        if (this == &rv)                                           \
            return *this;                                          \
        m_id = rv.m_id;                                            \
        f1 = std::move(rv.f1);                                     \
        rv.m_id = 0;                                               \
        return *this;                                              \
    }                                                              \
                                                                   \
private:                                                           \
    uint64_t m_id = 0;                                             \
                                                                   \
public:                                                            \
    bool valid() const {                                           \
        return m_id != 0;                                          \
    }                                                              \
    uint64_t id() const {                                          \
        return m_id;                                               \
    }                                                              \
    void id(uint64_t id) {                                         \
        m_id = id;                                                 \
    }                                                              \
    uint64_t rowid() const {                                       \
        return m_id;                                               \
    }                                                              \
    void rowid(uint64_t id) {                                      \
        m_id = id;                                                 \
    }                                                              \
    static std::string getTableName() {                            \
        return #table;                                             \
    }                                                              \
    static const char* getInsertSQL() {                            \
        return "insert into `" #table "` (`" #f1 "`) values (?)";  \
    }                                                              \
    static const char* getUpdateSQL() {                            \
        return "update `" #table "` set `" #f1 "`=? where `id`=?"; \
    }                                                              \
    static const char* getSelectSQL() {                            \
        return "select `id`,`" #f1 "` from `" #table "`";          \
    }                                                              \
    void save(const SQLStatementPtr& st) const {                   \
        st->bind(0, f1);                                           \
    }                                                              \
    void update(const SQLStatementPtr& st) const {                 \
        st->bind(0, f1, m_id);                                     \
    }                                                              \
    void load(const SQLStatementPtr& st) {                         \
        st->getColumn(0, m_id, f1);                                \
    }

#define TABLE_BIND2(TableT, table, f1, f2)                                            \
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
        rv.m_id = 0;                                                                  \
        f1 = std::move(rv.f1);                                                        \
        f2 = std::move(rv.f2);                                                        \
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
    static std::string getTableName() {                                               \
        return #table;                                                                \
    }                                                                                 \
    static const char* getInsertSQL() {                                               \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`) values (?,?)";         \
    }                                                                                 \
    static const char* getUpdateSQL() {                                               \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=? where `id`=?";        \
    }                                                                                 \
    static const char* getSelectSQL() {                                               \
        return "select `id`,`" #f1 "`,`" #f2 "` from `" #table "`";                   \
    }                                                                                 \
    void save(const SQLStatementPtr& st) const {                                      \
        st->bind(0, f1, f2);                                                          \
    }                                                                                 \
    void update(const SQLStatementPtr& st) const {                                    \
        st->bind(0, f1, f2, m_id);                                                    \
    }                                                                                 \
    void load(const SQLStatementPtr& st) {                                            \
        st->getColumn(0, m_id, f1, f2);                                               \
    }

#define TABLE_BIND3(TableT, table, f1, f2, f3)                                             \
public:                                                                                    \
    TableT() = default;                                                                    \
    TableT(const TableT&) = default;                                                       \
    TableT& operator=(const TableT&) = default;                                            \
    TableT(TableT&& rv)                                                                    \
    : m_id(rv.m_id), f1(std::move(rv.f1)), f2(std::move(rv.f2)), f3(std::move(rv.f3)) {    \
        rv.m_id = 0;                                                                       \
    }                                                                                      \
    TableT& operator=(TableT&& rv) {                                                       \
        if (this == &rv)                                                                   \
            return *this;                                                                  \
        m_id = rv.m_id;                                                                    \
        f1 = std::move(rv.f1);                                                             \
        f2 = std::move(rv.f2);                                                             \
        f3 = std::move(rv.f3);                                                             \
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
    static std::string getTableName() {                                                    \
        return #table;                                                                     \
    }                                                                                      \
    static const char* getInsertSQL() {                                                    \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`) values (?,?,?)";  \
    }                                                                                      \
    static const char* getUpdateSQL() {                                                    \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=? where `id`=?"; \
    }                                                                                      \
    static const char* getSelectSQL() {                                                    \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "` from `" #table "`";              \
    }                                                                                      \
    void save(const SQLStatementPtr& st) const {                                           \
        st->bind(0, f1, f2, f3);                                                           \
    }                                                                                      \
    void update(const SQLStatementPtr& st) const {                                         \
        st->bind(0, f1, f2, f3, m_id);                                                     \
    }                                                                                      \
    void load(const SQLStatementPtr& st) {                                                 \
        st->getColumn(0, m_id, f1, f2, f3);                                                \
    }

#define TABLE_BIND4(TableT, table, f1, f2, f3, f4)                                      \
public:                                                                                 \
    TableT() = default;                                                                 \
    TableT(const TableT&) = default;                                                    \
    TableT& operator=(const TableT&) = default;                                         \
    TableT(TableT&& rv)                                                                 \
    : m_id(rv.m_id),                                                                    \
      f1(std::move(rv.f1)),                                                             \
      f2(std::move(rv.f2)),                                                             \
      f3(std::move(rv.f3)),                                                             \
      f4(std::move(rv.f4)) {                                                            \
        rv.m_id = 0;                                                                    \
    }                                                                                   \
    TableT& operator=(TableT&& rv) {                                                    \
        if (this == &rv)                                                                \
            return *this;                                                               \
        m_id = rv.m_id;                                                                 \
        f1 = std::move(rv.f1);                                                          \
        f2 = std::move(rv.f2);                                                          \
        f3 = std::move(rv.f3);                                                          \
        f4 = std::move(rv.f4);                                                          \
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
    static std::string getTableName() {                                                 \
        return #table;                                                                  \
    }                                                                                   \
    static const char* getInsertSQL() {                                                 \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4          \
               "`) values (?,?,?,?)";                                                   \
    }                                                                                   \
    static const char* getUpdateSQL() {                                                 \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4      \
               "`=? where `id`=?";                                                      \
    }                                                                                   \
    static const char* getSelectSQL() {                                                 \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "` from `" #table "`"; \
    }                                                                                   \
    void save(const SQLStatementPtr& st) const {                                        \
        st->bind(0, f1, f2, f3, f4);                                                    \
    }                                                                                   \
    void update(const SQLStatementPtr& st) const {                                      \
        st->bind(0, f1, f2, f3, f4, m_id);                                              \
    }                                                                                   \
    void load(const SQLStatementPtr& st) {                                              \
        st->getColumn(0, m_id, f1, f2, f3, f4);                                         \
    }

#define TABLE_BIND5(TableT, table, f1, f2, f3, f4, f5)                                            \
public:                                                                                           \
    TableT() = default;                                                                           \
    TableT(const TableT&) = default;                                                              \
    TableT& operator=(const TableT&) = default;                                                   \
    TableT(TableT&& rv)                                                                           \
    : m_id(rv.m_id),                                                                              \
      f1(std::move(rv.f1)),                                                                       \
      f2(std::move(rv.f2)),                                                                       \
      f3(std::move(rv.f3)),                                                                       \
      f4(std::move(rv.f4)),                                                                       \
      f5(std::move(rv.f5)) {                                                                      \
        rv.m_id = 0;                                                                              \
    }                                                                                             \
    TableT& operator=(TableT&& rv) {                                                              \
        if (this == &rv)                                                                          \
            return *this;                                                                         \
        m_id = rv.m_id;                                                                           \
        f1 = std::move(rv.f1);                                                                    \
        f2 = std::move(rv.f2);                                                                    \
        f3 = std::move(rv.f3);                                                                    \
        f4 = std::move(rv.f4);                                                                    \
        f5 = std::move(rv.f5);                                                                    \
        rv.m_id = 0;                                                                              \
        return *this;                                                                             \
    }                                                                                             \
                                                                                                  \
private:                                                                                          \
    uint64_t m_id = 0;                                                                            \
                                                                                                  \
public:                                                                                           \
    bool valid() const {                                                                          \
        return m_id != 0;                                                                         \
    }                                                                                             \
    uint64_t id() const {                                                                         \
        return m_id;                                                                              \
    }                                                                                             \
    void id(uint64_t id) {                                                                        \
        m_id = id;                                                                                \
    }                                                                                             \
    uint64_t rowid() const {                                                                      \
        return m_id;                                                                              \
    }                                                                                             \
    void rowid(uint64_t id) {                                                                     \
        m_id = id;                                                                                \
    }                                                                                             \
    static std::string getTableName() {                                                           \
        return #table;                                                                            \
    }                                                                                             \
    static const char* getInsertSQL() {                                                           \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5          \
               "`) values (?,?,?,?,?)";                                                           \
    }                                                                                             \
    static const char* getUpdateSQL() {                                                           \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5    \
               "`=? where `id`=?";                                                                \
    }                                                                                             \
    static const char* getSelectSQL() {                                                           \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "` from `" #table "`"; \
    }                                                                                             \
    void save(const SQLStatementPtr& st) const {                                                  \
        st->bind(0, f1, f2, f3, f4, f5);                                                          \
    }                                                                                             \
    void update(const SQLStatementPtr& st) const {                                                \
        st->bind(0, f1, f2, f3, f4, f5, m_id);                                                    \
    }                                                                                             \
    void load(const SQLStatementPtr& st) {                                                        \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5);                                               \
    }

#define TABLE_BIND6(TableT, table, f1, f2, f3, f4, f5, f6)                                         \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)) {                                                                       \
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
        f6 = std::move(rv.f6);                                                                     \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`) values (?,?,?,?,?,?)";                                                          \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=? where `id`=?";                                                     \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6               \
               "` from `" #table "`";                                                              \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6);                                                       \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, m_id);                                                 \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6);                                            \
    }

#define TABLE_BIND7(TableT, table, f1, f2, f3, f4, f5, f6, f7)                                     \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)) {                                                                       \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`) values (?,?,?,?,?,?,?)";                                              \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=? where `id`=?";                                         \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "` from `" #table "`";                                                              \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7);                                                   \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, m_id);                                             \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7);                                        \
    }

#define TABLE_BIND8(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8)                                 \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)) {                                                                       \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`) values (?,?,?,?,?,?,?,?)";                                  \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=? where `id`=?";                             \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "` from `" #table "`";                                                    \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8);                                               \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, m_id);                                         \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8);                                    \
    }

#define TABLE_BIND9(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9)                             \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)) {                                                                       \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`) values (?,?,?,?,?,?,?,?,?)";                      \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=? where `id`=?";                 \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "` from `" #table "`";                                          \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9);                                           \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, m_id);                                     \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9);                                \
    }

#define TABLE_BIND10(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10)                       \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`) values (?,?,?,?,?,?,?,?,?,?)";         \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=? where `id`=?";    \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "` from `" #table "`";                               \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);                                      \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, m_id);                                \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);                           \
    }

#define TABLE_BIND11(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11)                  \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11                                 \
               "`) values (?,?,?,?,?,?,?,?,?,?,?)";                                                \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=? where `id`=?";                                                                 \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "` from `" #table "`";                    \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11);                                 \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, m_id);                           \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11);                      \
    }

#define TABLE_BIND12(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12)             \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12                      \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?)";                                              \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=? where `id`=?";                                                    \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "` from `" #table "`";         \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12);                            \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, m_id);                      \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12);                 \
    }

#define TABLE_BIND13(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13)        \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?)";                                            \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=? where `id`=?";                                       \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "` from `" #table   \
               "`";                                                                                \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13);                       \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, m_id);                 \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13);            \
    }

#define TABLE_BIND14(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14)   \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)),                                                                      \
      f14(std::move(rv.f14)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
        f14 = std::move(rv.f14);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                               \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=? where `id`=?";                          \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "` from `" #table "`";                                                              \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14);                  \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, m_id);            \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14);       \
    }

#define TABLE_BIND15(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14,   \
                     f15)                                                                          \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)),                                                                      \
      f14(std::move(rv.f14)),                                                                      \
      f15(std::move(rv.f15)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
        f14 = std::move(rv.f14);                                                                   \
        f15 = std::move(rv.f15);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                  \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=? where `id`=?";             \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "` from `" #table "`";                                                   \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15);             \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, m_id);       \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15);  \
    }

#define TABLE_BIND16(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14,   \
                     f15, f16)                                                                     \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)),                                                                      \
      f14(std::move(rv.f14)),                                                                      \
      f15(std::move(rv.f15)),                                                                      \
      f16(std::move(rv.f16)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
        f14 = std::move(rv.f14);                                                                   \
        f15 = std::move(rv.f15);                                                                   \
        f16 = std::move(rv.f16);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";     \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16                    \
               "`=? where `id`=?";                                                                 \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "` from `" #table "`";                                        \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16);        \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, m_id);  \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,   \
                      f16);                                                                        \
    }

#define TABLE_BIND17(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14,   \
                     f15, f16, f17)                                                                \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)),                                                                      \
      f14(std::move(rv.f14)),                                                                      \
      f15(std::move(rv.f15)),                                                                      \
      f16(std::move(rv.f16)),                                                                      \
      f17(std::move(rv.f17)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
        f14 = std::move(rv.f14);                                                                   \
        f15 = std::move(rv.f15);                                                                   \
        f16 = std::move(rv.f16);                                                                   \
        f17 = std::move(rv.f17);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17                                         \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                                    \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=? where `id`=?";                                                                 \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "` from `" #table "`";                             \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17);   \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 m_id);                                                                            \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,   \
                      f16, f17);                                                                   \
    }

#define TABLE_BIND18(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14,   \
                     f15, f16, f17, f18)                                                           \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)),                                                                      \
      f14(std::move(rv.f14)),                                                                      \
      f15(std::move(rv.f15)),                                                                      \
      f16(std::move(rv.f16)),                                                                      \
      f17(std::move(rv.f17)),                                                                      \
      f18(std::move(rv.f18)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
        f14 = std::move(rv.f14);                                                                   \
        f15 = std::move(rv.f15);                                                                   \
        f16 = std::move(rv.f16);                                                                   \
        f17 = std::move(rv.f17);                                                                   \
        f18 = std::move(rv.f18);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18                              \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                                  \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=?,`" #f18 "`=? where `id`=?";                                                    \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "` from `" #table "`";                  \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18);                                                                             \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, m_id);                                                                       \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,   \
                      f16, f17, f18);                                                              \
    }

#define TABLE_BIND19(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14,   \
                     f15, f16, f17, f18, f19)                                                      \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)),                                                                      \
      f14(std::move(rv.f14)),                                                                      \
      f15(std::move(rv.f15)),                                                                      \
      f16(std::move(rv.f16)),                                                                      \
      f17(std::move(rv.f17)),                                                                      \
      f18(std::move(rv.f18)),                                                                      \
      f19(std::move(rv.f19)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
        f14 = std::move(rv.f14);                                                                   \
        f15 = std::move(rv.f15);                                                                   \
        f16 = std::move(rv.f16);                                                                   \
        f17 = std::move(rv.f17);                                                                   \
        f18 = std::move(rv.f18);                                                                   \
        f19 = std::move(rv.f19);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19                   \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                                \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=?,`" #f18 "`=?,`" #f19 "`=? where `id`=?";                                       \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "` from `" #table "`";       \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19);                                                                        \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19, m_id);                                                                  \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,   \
                      f16, f17, f18, f19);                                                         \
    }

#define TABLE_BIND20(TableT, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14,   \
                     f15, f16, f17, f18, f19, f20)                                                 \
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
      f5(std::move(rv.f5)),                                                                        \
      f6(std::move(rv.f6)),                                                                        \
      f7(std::move(rv.f7)),                                                                        \
      f8(std::move(rv.f8)),                                                                        \
      f9(std::move(rv.f9)),                                                                        \
      f10(std::move(rv.f10)),                                                                      \
      f11(std::move(rv.f11)),                                                                      \
      f12(std::move(rv.f12)),                                                                      \
      f13(std::move(rv.f13)),                                                                      \
      f14(std::move(rv.f14)),                                                                      \
      f15(std::move(rv.f15)),                                                                      \
      f16(std::move(rv.f16)),                                                                      \
      f17(std::move(rv.f17)),                                                                      \
      f18(std::move(rv.f18)),                                                                      \
      f19(std::move(rv.f19)),                                                                      \
      f20(std::move(rv.f20)) {                                                                     \
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
        f6 = std::move(rv.f6);                                                                     \
        f7 = std::move(rv.f7);                                                                     \
        f8 = std::move(rv.f8);                                                                     \
        f9 = std::move(rv.f9);                                                                     \
        f10 = std::move(rv.f10);                                                                   \
        f11 = std::move(rv.f11);                                                                   \
        f12 = std::move(rv.f12);                                                                   \
        f13 = std::move(rv.f13);                                                                   \
        f14 = std::move(rv.f14);                                                                   \
        f15 = std::move(rv.f15);                                                                   \
        f16 = std::move(rv.f16);                                                                   \
        f17 = std::move(rv.f17);                                                                   \
        f18 = std::move(rv.f18);                                                                   \
        f19 = std::move(rv.f19);                                                                   \
        f20 = std::move(rv.f20);                                                                   \
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
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "`,`" #f20        \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                              \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=?,`" #f18 "`=?,`" #f19 "`=?,`" #f20 "`=? where `id`=?";                          \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7     \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "`,`" #f20 "` from `" #table \
               "`";                                                                                \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19, f20);                                                                   \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19, f20, m_id);                                                             \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,   \
                      f16, f17, f18, f20);                                                         \
    }

//-------------------------------------------------------------------
//
// 不带自增ID的表绑定
// ROWID：由于 MYSQL 和 SQLITE 语法不一样，需明确指定
//        MYSQL: _rowid
//        sqlite: rowid
//
//-------------------------------------------------------------------

#define TABLE_NO_AUTOID_BIND1(ROWID, table, f1)                            \
private:                                                                   \
    uint64_t m_rowid = 0;                                                  \
                                                                           \
public:                                                                    \
    bool valid() const {                                                   \
        return m_rowid != 0;                                               \
    }                                                                      \
    uint64_t rowid() const {                                               \
        return m_rowid;                                                    \
    }                                                                      \
    void rowid(uint64_t id) {                                              \
        m_rowid = id;                                                      \
    }                                                                      \
    static std::string getTableName() {                                    \
        return #table;                                                     \
    }                                                                      \
    static const char* getInsertSQL() {                                    \
        return "insert into `" #table "` (`" #f1 "`) values (?)";          \
    }                                                                      \
    static const char* getUpdateSQL() {                                    \
        return "update `" #table "` set `" #f1 "`=? where `" #ROWID "`=?"; \
    }                                                                      \
    static const char* getSelectSQL() {                                    \
        return "select `" #f1 "` from `" #table "`";                       \
    }                                                                      \
    void save(const SQLStatementPtr& st) const {                           \
        st->bind(0, f1);                                                   \
    }                                                                      \
    void update(const SQLStatementPtr& st) const {                         \
        st->bind(0, f1, m_rowid);                                          \
    }                                                                      \
    void load(const SQLStatementPtr& st) {                                 \
        st->getColumn(0, f1);                                              \
    }

#define TABLE_NO_AUTOID_BIND2(ROWID, table, f1, f2)                                   \
private:                                                                              \
    uint64_t m_rowid = 0;                                                             \
                                                                                      \
public:                                                                               \
    bool valid() const {                                                              \
        return m_rowid != 0;                                                          \
    }                                                                                 \
    uint64_t rowid() const {                                                          \
        return m_rowid;                                                               \
    }                                                                                 \
    void rowid(uint64_t id) {                                                         \
        m_rowid = id;                                                                 \
    }                                                                                 \
    static std::string getTableName() {                                               \
        return #table;                                                                \
    }                                                                                 \
    static const char* getInsertSQL() {                                               \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`) values (?,?)";         \
    }                                                                                 \
    static const char* getUpdateSQL() {                                               \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=? where `" ROWID "`=?"; \
    }                                                                                 \
    static const char* getSelectSQL() {                                               \
        return "select `" #f1 "`,`" #f2 "` from `" #table "`";                        \
    }                                                                                 \
    void save(const SQLStatementPtr& st) const {                                      \
        st->bind(0, f1, f2);                                                          \
    }                                                                                 \
    void update(const SQLStatementPtr& st) const {                                    \
        st->bind(0, f1, f2, m_rowid);                                                 \
    }                                                                                 \
    void load(const SQLStatementPtr& st) {                                            \
        st->getColumn(0, f1, f2);                                                     \
    }

#define TABLE_NO_AUTOID_BIND3(ROWID, table, f1, f2, f3)                                            \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`) values (?,?,?)";          \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=? where `" #ROWID "`=?"; \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "` from `" #table "`";                           \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3);                                                                   \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, m_rowid);                                                          \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3);                                                              \
    }

#define TABLE_NO_AUTOID_BIND4(ROWID, table, f1, f2, f3, f4)                        \
private:                                                                           \
    uint64_t m_rowid = 0;                                                          \
                                                                                   \
public:                                                                            \
    bool valid() const {                                                           \
        return m_rowid != 0;                                                       \
    }                                                                              \
    uint64_t rowid() const {                                                       \
        return m_rowid;                                                            \
    }                                                                              \
    void rowid(uint64_t id) {                                                      \
        m_rowid = id;                                                              \
    }                                                                              \
    static std::string getTableName() {                                            \
        return #table;                                                             \
    }                                                                              \
    static const char* getInsertSQL() {                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4     \
               "`) values (?,?,?,?)";                                              \
    }                                                                              \
    static const char* getUpdateSQL() {                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 \
               "`=? where `" #ROWID "`=?";                                         \
    }                                                                              \
    static const char* getSelectSQL() {                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "` from `" #table "`"; \
    }                                                                              \
    void save(const SQLStatementPtr& st) const {                                   \
        st->bind(0, f1, f2, f3, f4);                                               \
    }                                                                              \
    void update(const SQLStatementPtr& st) const {                                 \
        st->bind(0, f1, f2, f3, m_rowid);                                          \
    }                                                                              \
    void load(const SQLStatementPtr& st) {                                         \
        st->getColumn(0, f1, f2, f3);                                              \
    }

#define TABLE_NO_AUTOID_BIND5(ROWID, table, f1, f2, f3, f4, f5)                                \
private:                                                                                       \
    uint64_t m_rowid = 0;                                                                      \
                                                                                               \
public:                                                                                        \
    bool valid() const {                                                                       \
        return m_rowid != 0;                                                                   \
    }                                                                                          \
    uint64_t rowid() const {                                                                   \
        return m_rowid;                                                                        \
    }                                                                                          \
    void rowid(uint64_t id) {                                                                  \
        m_rowid = id;                                                                          \
    }                                                                                          \
    static std::string getTableName() {                                                        \
        return #table;                                                                         \
    }                                                                                          \
    static const char* getInsertSQL() {                                                        \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5       \
               "`) values (?,?,?,?,?)";                                                        \
    }                                                                                          \
    static const char* getUpdateSQL() {                                                        \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5 \
               "`=? where `" #ROWID "`=?";                                                     \
    }                                                                                          \
    static const char* getSelectSQL() {                                                        \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "` from `" #table "`";   \
    }                                                                                          \
    void save(const SQLStatementPtr& st) const {                                               \
        st->bind(0, f1, f2, f3, f4, f5);                                                       \
    }                                                                                          \
    void update(const SQLStatementPtr& st) const {                                             \
        st->bind(0, f1, f2, f3, f4, f5, m_rowid);                                              \
    }                                                                                          \
    void load(const SQLStatementPtr& st) {                                                     \
        st->getColumn(0, f1, f2, f3, f4, f5);                                                  \
    }

#define TABLE_NO_AUTOID_BIND6(ROWID, table, f1, f2, f3, f4, f5, f6)                                \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`) values (?,?,?,?,?,?)";                                                          \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=? where `" #ROWID "`=?";                                             \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "` from `" #table  \
               "`";                                                                                \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6);                                                       \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, m_rowid);                                              \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, m_rowid, f1, f2, f3, f4, f5, f6);                                         \
    }

#define TABLE_NO_AUTOID_BIND7(ROWID, table, f1, f2, f3, f4, f5, f6, f7)                            \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`) values (?,?,?,?,?,?,?)";                                              \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=? where `" #ROWID "`=?";                                 \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "` from `" #table "`";                                                              \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7);                                                   \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, m_rowid);                                          \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7);                                              \
    }

#define TABLE_NO_AUTOID_BIND8(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8)                        \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`) values (?,?,?,?,?,?,?,?)";                                  \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=? where `" #ROWID "`=?";                     \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "` from `" #table "`";                                                    \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8);                                               \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, m_rowid);                                      \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8);                                          \
    }

#define TABLE_NO_AUTOID_BIND9(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9)                    \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`) values (?,?,?,?,?,?,?,?,?)";                      \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=? where `" #ROWID "`=?";         \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "` from `" #table "`";                                          \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9);                                           \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, m_rowid);                                  \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9);                                      \
    }

#define TABLE_NO_AUTOID_BIND10(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10)              \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`) values (?,?,?,?,?,?,?,?,?,?)";         \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=? where `" #ROWID   \
               "`=?";                                                                              \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "` from `" #table "`";                               \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);                                      \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, m_rowid);                             \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);                                 \
    }

#define TABLE_NO_AUTOID_BIND11(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11)         \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11                                 \
               "`) values (?,?,?,?,?,?,?,?,?,?,?)";                                                \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=? where `" #ROWID "`=?";                                                         \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "` from `" #table "`";                    \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11);                                 \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, m_rowid);                        \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11);                            \
    }

#define TABLE_NO_AUTOID_BIND12(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12)    \
    pprivate : uint64_t m_rowid = 0;                                                               \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12                      \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?)";                                              \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=? where `" #ROWID "`=?";                                            \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "` from `" #table "`";         \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12);                            \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, m_rowid);                   \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12);                       \
    }

#define TABLE_NO_AUTOID_BIND13(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13)                                                                \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?)";                                            \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=? where `" #ROWID "`=?";                               \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "` from `" #table   \
               "`";                                                                                \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13);                       \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, m_rowid);              \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13);                  \
    }

#define TABLE_NO_AUTOID_BIND14(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13, f14)                                                           \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                               \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=? where `" #ROWID "`=?";                  \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "` from `" #table "`";                                                              \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14);                  \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, m_rowid);         \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14);             \
    }

#define TABLE_NO_AUTOID_BIND15(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13, f14, f15)                                                      \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                  \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=? where `" #ROWID "`=?";     \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "` from `" #table "`";                                                   \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15);             \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, m_rowid);    \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15);        \
    }

#define TABLE_NO_AUTOID_BIND16(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13, f14, f15, f16)                                                 \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";     \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16                    \
               "`=? where `" #ROWID "`=?";                                                         \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "` from `" #table "`";                                        \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16);        \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,         \
                 m_rowid);                                                                         \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16);   \
    }

#define TABLE_NO_AUTOID_BIND17(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13, f14, f15, f16, f17)                                            \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17                                         \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                                    \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=? where `" #ROWID "`=?";                                                         \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "` from `" #table "`";                             \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17);   \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 m_rowid);                                                                         \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,    \
                      f17);                                                                        \
    }

#define TABLE_NO_AUTOID_BIND18(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13, f14, f15, f16, f17, f18)                                       \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18                              \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                                  \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=?,`" #f18 "`=? where `" #ROWID "`=?";                                            \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "` from `" #table "`";                  \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18);                                                                             \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, m_rowid);                                                                    \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,    \
                      f17, f18);                                                                   \
    }

#define TABLE_NO_AUTOID_BIND19(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13, f14, f15, f16, f17, f18, f19)                                  \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19                   \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                                \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=?,`" #f18 "`=?,`" #f19 "`=? where `" #ROWID "`=?";                               \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "` from `" #table "`";       \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19);                                                                        \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19, m_rowid);                                                               \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,    \
                      f17, f18, f19);                                                              \
    }

#define TABLE_NO_AUTOID_BIND20(ROWID, table, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,    \
                               f13, f14, f15, f16, f17, f18, f19, f20)                             \
private:                                                                                           \
    uint64_t m_rowid = 0;                                                                          \
                                                                                                   \
public:                                                                                            \
    bool valid() const {                                                                           \
        return m_rowid != 0;                                                                       \
    }                                                                                              \
    uint64_t rowid() const {                                                                       \
        return m_rowid;                                                                            \
    }                                                                                              \
    void rowid(uint64_t id) {                                                                      \
        m_rowid = id;                                                                              \
    }                                                                                              \
    static std::string getTableName() {                                                            \
        return #table;                                                                             \
    }                                                                                              \
    static const char* getInsertSQL() {                                                            \
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 \
               "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13           \
               "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "`,`" #f20        \
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";                              \
    }                                                                                              \
    static const char* getUpdateSQL() {                                                            \
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=?,`" #f5     \
               "`=?,`" #f6 "`=?,`" #f7 "`=?,`" #f8 "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11           \
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17       \
               "`=?,`" #f18 "`=?,`" #f19 "`=?,`" #f20 "`=? where `" #ROWID "`=?";                  \
    }                                                                                              \
    static const char* getSelectSQL() {                                                            \
        return "select `" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`,`" #f6 "`,`" #f7          \
               "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12 "`,`" #f13 "`,`" #f14          \
               "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "`,`" #f20 "` from `" #table \
               "`";                                                                                \
    }                                                                                              \
    void save(const SQLStatementPtr& st) const {                                                   \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19, f20);                                                                   \
    }                                                                                              \
    void update(const SQLStatementPtr& st) const {                                                 \
        st->bind(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,    \
                 f18, f19, f20, m_rowid);                                                          \
    }                                                                                              \
    void load(const SQLStatementPtr& st) {                                                         \
        st->getColumn(0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,    \
                      f17, f18, f20);                                                              \
    }

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_TABLE_MACRO_H */