/*
 * StockTypeInfoTable.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKTYPEINFOTABLE_H
#define HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKTYPEINFOTABLE_H

#include "../../../utilities/db_connect/SQLStatementBase.h"

namespace hku {

class StockTypeInfoTable {
public:
    StockTypeInfoTable()
    : m_id(0),
      m_type(Null<uint32_t>()),
      m_precision(0),
      m_tick(0.0),
      m_tickValue(0.0),
      m_minTradeNumber(0),
      m_maxTradeNumber(0) {}

    int64_t id() const {
        return m_id;
    }

    uint32_t type() const {
        return m_type;
    }

    uint32_t precision() const {
        return m_precision;
    }

    double tick() const {
        return m_tick;
    }

    double tickValue() const {
        return m_tickValue;
    }

    double minTradeNumber() const {
        return m_minTradeNumber;
    }

    double maxTradeNumber() const {
        return m_maxTradeNumber;
    }

    const string& description() const {
        return m_description;
    }

public:
    static const char* getInsertSQL() {
        return "insert into `stocktypeinfo` "
               "(`id`, `type`, `precision`, `tick`, `tickValue`, "
               "`minTradeNumber`, `maxTradeNumber`, `description`) "
               "values (?,?,?,?,?,?,?,?)";
    }

    static const char* getUpdateSQL() {
        return "update `stocktypeinfo` set `type`=?, `precision`=?, `tick`=?, "
               "`tickValue`=?, `minTradeNumber`=?, `maxTradeNumber`=?"
               "`description`=? where `id`=?";
    }

    static const char* getSelectSQL() {
        return "select `id`,`type`,`precision`, `tick`, `tickValue`, "
               "`minTradeNumber`, `maxTradeNumber`, `description` from `stocktypeinfo`";
    }

    void save(const SQLStatementPtr& st) const {
        st->bind(0, m_id, m_type, m_precision, m_tick, m_tickValue, m_minTradeNumber,
                 m_maxTradeNumber, m_description);
    }

    void update(const SQLStatementPtr& st) const {
        st->bind(0, m_type, m_precision, m_tick, m_tickValue, m_minTradeNumber, m_maxTradeNumber,
                 m_description, m_id);
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, m_id, m_type, m_precision, m_tick, m_tickValue, m_minTradeNumber,
                      m_maxTradeNumber, m_description);
    }

private:
    int64_t m_id;
    uint32_t m_type;          //证券类型
    uint32_t m_precision;     //价格精度
    double m_tick;            //最小跳动量
    double m_tickValue;       //每一个tick价格
    double m_minTradeNumber;  //每笔最小交易量
    double m_maxTradeNumber;  //每笔最大交易量
    string m_description;     //描述信息
};

}  // namespace hku

#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKTYPEINFOTABLE_H */
