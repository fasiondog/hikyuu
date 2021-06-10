/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-11
 *     Author: fasiondog
 */

#include <string>
#include <string.h>
#include <random>
#include "common/log.h"
#include "BCrypt.hpp"
#include "pycabcrypt.h"

namespace hku {

void BCrypt::urandom(char *buf, int len) {
    std::random_device rd;                    // 产生一个 std::random_device 对象 rd
    std::default_random_engine random(rd());  // 用 rd 初始化一个随机数发生器 random

    char tmp;
    for (int i = 0; i < len; i++) {
        tmp = random() % 36;  // 随机一个小于 36 的整数，0-9、A-Z 共 36 种字符
        if (tmp < 10) {       // 如果随机数小于 10，变换成一个阿拉伯数字的 ASCII
            tmp += '0';
        } else {  // 否则，变换成一个大写字母的 ASCII
            tmp -= 10;
            tmp += 'A';
        }
        buf[i] = tmp;
    }
}

std::string BCrypt::gensalt(int rounds, const char *prefix) {
    APP_CHECK(rounds >= 4 && rounds <= 31, "Invalid rounds");
    APP_CHECK(strcmp(prefix, "2a") == 0 || strcmp(prefix, "2b") == 0,
              "Supported prefixes are b'2a' or b'2b'");

    std::string salt;
    char str_random[16];
    urandom(str_random, 16);
    char output[30] = {0};
    encode_base64(output, (const u_int8_t *)str_random, 16);
    return fmt::format("${}${}${}", prefix, rounds, output);
}

std::string BCrypt::hashpw(const std::string &password, const std::string &salt) {
    std::string pwd;
    if (password.size() > 72) {
        pwd = password.substr(0, 72);
    } else {
        pwd = password;
    }

    char hashed[128] = {0};
    APP_CHECK(bcrypt_hashpass(pwd.c_str(), salt.c_str(), hashed, 128) == 0, "Invalid salt");

    return std::string(hashed);
}

bool BCrypt::checkpw(const std::string &password, const std::string &hashed_password) {
    auto ret = hashpw(password, hashed_password);
    if (ret.size() != hashed_password.size()) {
        return false;
    }
    return timingsafe_bcmp(ret.c_str(), hashed_password.c_str(), hashed_password.size());
}

}  // namespace hku