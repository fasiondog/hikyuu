/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-11
 *     Author: fasiondog
 */

#pragma once

namespace hku {

class BCrypt {
public:
    static std::string gensalt(int rounds = 12, const char *prefix = "2b");
    static std::string hashpw(const std::string &password, const std::string &salt);
    static bool checkpw(const std::string &password, const std::string &hashed_password);

private:
    /*
     * 生成随机字符窗
     * @param buf 输出缓冲区地址
     * @param len 输出缓冲区长度
     */
    static void urandom(char *buf, int len);
};

}  // namespace hku