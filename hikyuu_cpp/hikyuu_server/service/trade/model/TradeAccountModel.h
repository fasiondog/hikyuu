/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-11
 *     Author: fasiondog
 */

#pragma once

#include <hikyuu/utilities/db_connect/DBConnect.h>

namespace hku {

class TradeAccountModel {
    TABLE_BIND3(td_account, td_id, name, type)

public:
    static bool isExistName(DBConnectPtr con, const std::string& name) {
        SQLStatementPtr st = con->getStatement(
          fmt::format(R"(select count(id) from {} where name="{}")", getTableName(), name));
        st->exec();
        st->moveNext();
        int result = 0;
        st->getColumn(0, result);
        return result != 0;
    }

public:
    int64_t getTdId() const {
        return td_id;
    }

    void setTdId(int64_t id) {
        td_id = id;
    }

    std::string getName() const {
        return name;
    }

    void setName(const std::string& name) {
        this->name = name;
    }

    std::string getType() const {
        return type;
    }

    void setType(const std::string& type) {
        this->type = type;
    }

    string json() const {
        return fmt::format(R"({{"td_id":"{}", "name":"{}", "type":"{}"}})", td_id, name, type);
    }

private:
    int64_t td_id;  // 内部交易账户id
    string name;    // 内部交易账户名称
    string type;    // 内部交易账户类型：xq（雪球模拟账户）
};

inline std::ostream& operator<<(std::ostream& out, const TradeAccountModel& model) {
    string strip(", ");
    out << "(" << model.id() << strip << model.getTdId() << strip << model.getName() << strip
        << model.getType() << ")";
    return out;
}

}  // namespace hku