/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-11
 *     Author: fasiondog
 */

#pragma once

#include <nlohmann/json.hpp>
#include <hikyuu/utilities/db_connect/TableMacro.h>

using nlohmann::json;

namespace hku {

class TradeAccountModel {
    TABLE_BIND3(td_account, account, name, type)

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
    j = json{{"id", p.m_id}, {"account", p.account}, {"name", p.name}, {"type", p.type}};
}

inline void from_json(const json& j, TradeAccountModel& p) {
    j.at("id").get_to(p.m_id);
    j.at("account").get_to(p.account);
    j.at("name").get_to(p.name);
    j.at("type").get_to(p.type);
}

}  // namespace hku