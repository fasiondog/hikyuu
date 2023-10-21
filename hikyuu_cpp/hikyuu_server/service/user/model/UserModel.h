/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-24
 *     Author: fasiondog
 */

#pragma once

#include <string_view>
#include <hikyuu/utilities/db_connect/TableMacro.h>
#include "bcrypt/BCrypt.hpp"

namespace hku {

class UserModel {
    TABLE_BIND6(UserModel, admin_user, userid, name, password, start_time, end_time, status)

    enum STATUS {
        NORMAL = 1,    // 正常用户
        DELETED = 99,  // 已删除用户，数据库不实际执行删除操作，仅做删除标记
    };

public:
    uint64_t getUserId() const {
        return userid;
    }

    void setUserId(uint64_t id) {
        userid = id;
    }

    const std::string& getName() const {
        return name;
    }

    void setName(const std::string& name) {
        this->name = name;
    }

    const std::string& getPassword() const {
        return password;
    }

    void setPassword(const std::string& password) {
        auto salt = BCrypt::gensalt(12, "2b");
        this->password = BCrypt::hashpw(password, salt);
    }

    bool checkPassword(const std::string& password) {
        return BCrypt::checkpw(password, this->password);
    }

    Datetime getStartTime() const {
        return Datetime(start_time);
    }

    void setStartTime(Datetime time) {
        start_time = time.number();
    }

    Datetime getEndTime() const {
        return Datetime(end_time);
    }

    void setEndTime(Datetime time) {
        end_time = time.number();
    }

    STATUS getStatus() const {
        return static_cast<STATUS>(status);
    }

    void setStatus(STATUS status) {
        this->status = status;
    }

private:
    uint64_t userid{0};                           // 用户id
    std::string name;                             // 用户名
    std::string password;                         // 用户密码
    uint64_t start_time;                          // 用户创建时间，精确到分
    uint64_t end_time{Datetime::max().number()};  // 用户失效时间，精确到分
    int status{STATUS::NORMAL};                   // 用户状态
};

}  // namespace hku