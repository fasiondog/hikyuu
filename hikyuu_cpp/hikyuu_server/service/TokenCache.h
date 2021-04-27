/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-27
 *     Author: fasiondog
 */

#pragma once

#include <unordered_set>
#include <mutex>
#include <string>
#include "db/db.h"

namespace hku {

class TokenCache {
public:
    TokenCache() = delete;

    static void put(const std::string& token) {
        std::lock_guard<std::mutex> lock(ms_mutex);
        ms_dict.insert(token);
    }

    static bool have(const std::string& token) {
        std::lock_guard<std::mutex> lock(ms_mutex);
        return ms_dict.find(token) != ms_dict.end();
    }

    static void remove(const std::string& token) {
        std::lock_guard<std::mutex> lock(ms_mutex);
        ms_dict.erase(token);
    }

private:
    static std::mutex ms_mutex;
    static std::unordered_set<std::string> ms_dict;
};

}  // namespace hku