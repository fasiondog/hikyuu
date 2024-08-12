/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "url.h"

namespace hku {

#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_NUM(c) ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c) (IS_ALPHA(c) || IS_NUM(c))
#define IS_HEX(c) (IS_NUM(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))

static inline bool is_unambiguous(char c) {
    return IS_ALPHANUM(c) || c == '-' || c == '_' || c == '.' || c == '~';
}

static inline unsigned char hex2i(char hex) {
    return hex <= '9' ? hex - '0' : hex <= 'F' ? hex - 'A' + 10 : hex - 'a' + 10;
}

std::string url_escape(const char* istr) {
    std::string ostr;
    const char* p = istr;
    char szHex[4] = {0};
    while (*p != '\0') {
        if (is_unambiguous(*p)) {
            ostr += *p;
        } else {
            snprintf(szHex, 4, "%%%02X", *p);
            ostr += szHex;
        }
        ++p;
    }
    return ostr;
}

std::string url_unescape(const char* istr) {
    std::string ostr;
    const char* p = istr;
    while (*p != '\0') {
        if (*p == '%' && IS_HEX(p[1]) && IS_HEX(p[2])) {
            ostr += ((hex2i(p[1]) << 4) | hex2i(p[2]));
            p += 3;
        } else {
            ostr += *p;
            ++p;
        }
    }
    return ostr;
}

}  // namespace hku