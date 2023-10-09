/*
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2020-9-27
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DATA_DRIVER_KDATA_MYSQL_KRECORDTABLE_H
#define HIKYUU_DATA_DRIVER_KDATA_MYSQL_KRECORDTABLE_H

#include "../../../KQuery.h"
#include "../../../utilities/db_connect/SQLStatementBase.h"

namespace hku {

class KRecordTable {
public:
    KRecordTable()
    : m_date(0), m_open(0.0), m_high(0.0), m_low(0.0), m_close(0.0), m_amount(0.0), m_count(0.0) {}

    KRecordTable(const string& market, const string& code, const KQuery::KType& ktype)
    : m_db_name(fmt::format("{}_{}", market, KQuery::getKTypeName(ktype))),
      m_code(code),
      m_date(0),
      m_open(0.0),
      m_high(0.0),
      m_low(0.0),
      m_close(0.0),
      m_amount(0.0),
      m_count(0.0) {
        // m_db_name = fmt::format("{}_{}", market, KQuery::getKTypeName(ktype));
        to_lower(m_db_name);
    };

    Datetime date() const {
        return m_date == 0 ? Null<Datetime>() : Datetime((uint64_t)m_date);
    }

    price_t open() const {
        return m_open;
    }

    price_t high() const {
        return m_high;
    }

    price_t low() const {
        return m_low;
    }

    price_t close() const {
        return m_close;
    }

    price_t amount() const {
        return m_amount;
    }

    price_t count() const {
        return m_count;
    }

    string str() const {
        return fmt::format(
          "KRecordTable({}(date), {}(open), {}(high), {}(low), {}(close), {}(amount), {}(count))",
          m_date, m_open, m_high, m_low, m_close, m_amount, m_count);
    }

public:
    string getInsertSQL() {
        return fmt::format(
          "insert into `{}`.`{}` "
          "(`date`, `open`, `high`, `low`, `close`, `amount`, `count`) "
          "values (?,?,?,?,?,?,?)",
          m_db_name, m_code);
    }

    string getUpdateSQL() {
        return fmt::format(
          "update `{}`.`{}` set `open`=?, `high`=?, `low`=?, "
          "`close`=?, `amount`=? `count`=? where `date`=?",
          m_db_name, m_code);
    }

    string getSelectSQL() {
        return fmt::format(
          "select `date`,`open`,`high`, `low`, `close`, `amount`, `count` from `{}`.`{}`",
          m_db_name, m_code);
    }

    string getSelectSQLNoDB() {
        return fmt::format(
          "select `date`,`open`,`high`, `low`, `close`, `amount`, `count` from `{}`",
          m_code);
    }

    void save(const SQLStatementPtr& st) const {
        st->bind(0, m_date, m_open, m_high, m_low, m_close, m_amount, m_count);
    }

    void update(const SQLStatementPtr& st) const {
        st->bind(0, m_open, m_high, m_low, m_close, m_amount, m_count);
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, m_date, m_open, m_high, m_low, m_close, m_amount, m_count);
    }

private:
    string m_db_name;
    string m_code;

    int64_t m_date;
    price_t m_open;
    price_t m_high;
    price_t m_low;
    price_t m_close;
    price_t m_amount;
    price_t m_count;
};

}  // namespace hku

#endif /* HIKYUU_DATA_DRIVER_KDATA_MYSQL_KRECORDTABLE_H */
