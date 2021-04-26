/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-27
 *     Author: fasiondog
 */

#pragma once

#include <unordered_map>
#include <mutex>
#include <string>
#include <hikyuu/datetime/Datetime.h>
#include "db/db.h"

namespace hku {

class TokenCache {
public:
    TokenCache() = delete;

    enum TOKEN_STATUS { INVALID = 0, VALID = 1, EXPIRED = 2 };

    static void add(const std::string& token, Datetime expired) {
        std::lock_guard<std::mutex> lock(ms_mutex);
        ms_dict[token] = expired;
    }

    static TOKEN_STATUS status(const std::string& token) {
        std::lock_guard<std::mutex> lock(ms_mutex);
        auto iter = ms_dict.find(token);
        if (iter == ms_dict.end()) {
            return TOKEN_STATUS::INVALID;
        }

        return iter->second <= Datetime::now() ? TOKEN_STATUS::VALID : TOKEN_STATUS::EXPIRED;
    }

    static Datetime getExpireTime(const std::string& token) {
        std::lock_guard<std::mutex> lock(ms_mutex);
        auto iter = ms_dict.find(token);
        return iter != ms_dict.end() ? iter->second : Datetime();
    }

private:
    static std::mutex ms_mutex;
    static std::unordered_map<std::string, Datetime> ms_dict;
};

}  // namespace hku