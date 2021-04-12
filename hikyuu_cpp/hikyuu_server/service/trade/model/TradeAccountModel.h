/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-11
 *     Author: fasiondog
 */

#pragma once

#include <nlohmann/json.hpp>
#include <hikyuu/utilities/db_connect/DBConnect.h>

using nlohmann::json;

namespace hku {

class TradeAccountModel {
    TABLE_BIND3(td_account, account, name, type)

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
    std::string getAccount() const {
        return account;
    }

    void setAccount(const std::string& account) {
        this->account = account;
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

private:
    string account;
    string name;
    string type;

    friend void to_json(json& j, const TradeAccountModel& p);
    friend void from_json(const json& j, TradeAccountModel& p);
};

inline void to_json(json& j, const TradeAccountModel& p) {
    j = json{{"account", p.account}, {"name", p.name}, {"type", p.type}};
}

inline void from_json(const json& j, TradeAccountModel& p) {
    j.at("account").get_to(p.account);
    j.at("name").get_to(p.name);
    j.at("type").get_to(p.type);
}

}  // namespace hku