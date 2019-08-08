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

#define TABLE_BIND1(table, f1) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`) values (?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0, f1, m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1); \
    }

#define TABLE_BIND2(table, f1, f2) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`) values (?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1, f2); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0, f1, f2, m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2); \
    }

#define TABLE_BIND3(table, f1, f2, f3) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`) values (?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3"` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1, f2, f3); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0, f1, f2, f3, m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1, f2, f3);\
    }

#define TABLE_BIND4(table, f1, f2, f3, f4) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`) values (?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4 "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0,m_id,f1,f2,f3,f4); \
    }

#define TABLE_BIND5(table, f1, f2, f3, f4, f5) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "`) values (?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5); \
    }

#define TABLE_BIND6(table, f1,f2,f3,f4,f5,f6) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 \
               "`) values (?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
                "`=?,`" #f6 \
                "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 \
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6); \
    }

#define TABLE_BIND7(table, f1,f2,f3,f4,f5,f6,f7) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7\
               "`) values (?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7 \
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7); \
    }

#define TABLE_BIND8(table, f1,f2,f3,f4,f5,f6,f7,f8) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8\
               "`) values (?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8 \
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8); \
    }

#define TABLE_BIND9(table, f1,f2,f3,f4,f5,f6,f7,f8,f9) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9\
               "`) values (?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 \
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9); \
    }

#define TABLE_BIND10(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10\
               "`) values (?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10); \
    }

#define TABLE_BIND11(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11\
               "`) values (?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11); \
    }

#define TABLE_BIND12(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12); \
    }

#define TABLE_BIND13(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13); \
    }

#define TABLE_BIND14(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14); \
    }

#define TABLE_BIND15(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15); \
    }

#define TABLE_BIND16(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16); \
    }

#define TABLE_BIND17(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17); \
    }

#define TABLE_BIND18(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17\
               "`=?,`" #f18\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18); \
    }

#define TABLE_BIND19(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17\
               "`=?,`" #f18 "`=?,`" #f19\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19); \
    }

#define TABLE_BIND20(table, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19,f20) \
private: \
    int64 m_id = 0; \
public: \
    int64 id() const { return m_id; } \
    static const char * getInsertSQL() {\
        return "insert into `" #table "` (`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "`,`" #f20\
               "`) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";\
    }\
    static const char * getUpdateSQL() {\
        return "update `" #table "` set `" #f1 "`=?,`" #f2 "`=?,`" #f3 "`=?,`" #f4  "`=?,`" #f5 \
               "`=?,`" #f6 "`=?,`" #f7  "`=?,`" #f8  "`=?,`" #f9 "`=?,`" #f10 "`=?,`" #f11\
               "`=?,`" #f12 "`=?,`" #f13 "`=?,`" #f14 "`=?,`" #f15 "`=?,`" #f16 "`=?,`" #f17\
               "`=?,`" #f18 "`=?,`" #f19 "`=?,`" #f20\
               "`=? where `id`=?";\
    }\
    static const char * getSelectSQL() {\
        return "select `id`,`" #f1 "`,`" #f2 "`,`" #f3 "`,`" #f4 "`,`" #f5 \
               "`,`" #f6 "`,`" #f7 "`,`" #f8 "`,`" #f9 "`,`" #f10 "`,`" #f11 "`,`" #f12\
               "`,`" #f13 "`,`" #f14 "`,`" #f15 "`,`" #f16 "`,`" #f17 "`,`" #f18 "`,`" #f19 "`,`" #f20\
               "` from `" #table "`";\
    }\
    void save(const SQLStatementPtr& st) const { \
        st->bind(0, f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19,f20); \
    }\
    void update(const SQLStatementPtr& st) const { \
        st->bind(0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19,f20,m_id); \
    }\
    void load(const SQLStatementPtr& st) {\
        st->getColumn(0, m_id,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f20); \
    }

} /* namespace */

#endif /* HIKYUU_DB_CONNECT_TABLE_MACRO_H */